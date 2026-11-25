import importlib.util
from pathlib import Path
import tempfile
import unittest
import json

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('flytec_common_stage', ROOT/'Common/stage.py')
common = importlib.util.module_from_spec(spec)
spec.loader.exec_module(common)

class CommonSourceTest(unittest.TestCase):
    def setup_root(self, root, sides=('Boat', 'Remote')):
        (root/'Common/source').mkdir(parents=True)
        (root/'Common/source/shared.h').write_bytes(b'original\n')
        for side in sides:
            (root/side/'source').mkdir(parents=True)
            (root/side/'platformio.ini').touch()
            (root/side/'source/local.h').write_bytes(b'endpoint\n')

    def test_clean_stage_update_and_retire(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); self.setup_root(root)
            common.stage(root); common.stage(root,check=True)
            (root/'Common/source/shared.h').write_bytes(b'new\n')
            with self.assertRaises(RuntimeError): common.stage(root,check=True)
            common.stage(root); common.stage(root,check=True)
            (root/'Common/source/new.h').write_bytes(b'new module\n')
            (root/'Common/source/shared.h').unlink()
            common.stage(root)
            for side in common.SIDES:
                self.assertFalse((root/side/'source/shared.h').exists())
                self.assertEqual((root/side/'source/new.h').read_bytes(),b'new module\n')
                self.assertEqual((root/side/'source/local.h').read_bytes(),b'endpoint\n')

    def test_edits_rejected_before_other_endpoint_changes(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);self.setup_root(root);common.stage(root)
            (root/'Remote/source/shared.h').write_bytes(b'local edit\n')
            (root/'Common/source/shared.h').write_bytes(b'new\n')
            with self.assertRaises(RuntimeError):common.stage(root)
            self.assertEqual((root/'Boat/source/shared.h').read_bytes(),b'original\n')
            self.assertEqual((root/'Remote/source/shared.h').read_bytes(),b'local edit\n')

    def test_standalone_endpoint_and_first_stage_collision(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);self.setup_root(root,('Remote',))
            (root/'Remote/source/shared.h').write_bytes(b'collision\n')
            with self.assertRaises(RuntimeError):common.stage(root)
            (root/'Remote/source/shared.h').unlink();common.stage(root);common.stage(root,check=True)
            self.assertFalse((root/'Boat').exists())

    def test_platformio_pre_script_without_file_global(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);self.setup_root(root,('Boat',))
            class Env:
                def subst(self, value):
                    self_value=value
                    if self_value!='$PROJECT_DIR':raise ValueError(self_value)
                    return str(root/'Boat')
            namespace={'__name__':'scons_extra_script'}
            def inject(name):
                self.assertEqual(name,'env');namespace[name]=Env()
            namespace['Import']=inject
            exec(compile((ROOT/'Common/stage.py').read_text(),'stage.py','exec'),namespace)
            self.assertEqual((root/'Boat/source/shared.h').read_bytes(),b'original\n')
            common.stage(root,check=True)

    def test_actual_source_and_generated_paths(self):
        expected=common.stage(ROOT,check=True)
        ignores=(ROOT/'.gitignore').read_text().splitlines()
        for side in common.SIDES:
            for name in expected:
                self.assertIn(f'/{side}/source/{name}',ignores)
            self.assertIn('extra_scripts = pre:../Common/stage.py',(ROOT/side/'platformio.ini').read_text())
            self.assertIn(f'/{side}/source/{common.STAMP}',ignores)

if __name__=='__main__':unittest.main()
