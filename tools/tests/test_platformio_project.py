import sys
import json
import subprocess
from pathlib import Path
import tempfile
import unittest
import zipfile

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/"tools"))
import flytec

class ProjectSnapshotTest(unittest.TestCase):
    def make_repo(self,root:Path)->None:
        subprocess.run(["git","init","-q"],cwd=root,check=True)
        (root/"Boat").mkdir()
        (root/"Boat"/"firmware.cpp").write_text("// boat\n",encoding="utf-8")
        (root/"Remote").mkdir()
        (root/"Remote"/"firmware.ino").write_text("// remote\n",encoding="utf-8")
        (root/"docs").mkdir()
        (root/"docs"/"README.md").write_text("# docs\n",encoding="utf-8")
        (root/"untracked.txt").write_text("private",encoding="utf-8")
        subprocess.run(["git","add","Boat","Remote","docs"],cwd=root,check=True)
        subprocess.run(["git","-c","user.name=CI","-c","user.email=ci@example.invalid",
                        "commit","-qm","fixture"],cwd=root,check=True)
    def test_archive_includes_every_tracked_file_and_no_untracked_file(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);self.make_repo(root)
            target=root/"out/source.zip"
            count,first=flytec.snapshot(root,target)
            self.assertEqual(count,3)
            with zipfile.ZipFile(target) as archive:
                names=set(archive.namelist())
                self.assertEqual(names,{"Boat/firmware.cpp","Remote/firmware.ino",
                                        "docs/README.md","FLYTEC_SOURCE_MANIFEST.json"})
                manifest=json.loads(archive.read("FLYTEC_SOURCE_MANIFEST.json"))
                self.assertEqual(manifest["file_count"],3)
                self.assertEqual(len(manifest["files"]),3)
                self.assertEqual(manifest["purpose"],"SOURCE_ONLY_NOT_SIGNED_FIRMWARE")
            count2,second=flytec.snapshot(root,target)
            self.assertEqual((count,first),(count2,second))
            self.assertTrue(target.with_name(target.name+".sha256").exists())
    def test_untracked_files_not_packaged_and_missing_tracked_files_fail(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);self.make_repo(root)
            (root/"Boat/firmware.cpp").unlink()
            with self.assertRaises(FileNotFoundError):flytec.snapshot(root,root/"out/snapshot.zip")
    def test_workspace_validation_under_current_branch(self):
        from verify_platformio_unified import verify
        self.assertEqual(verify(ROOT),[])

if __name__=="__main__":unittest.main()
