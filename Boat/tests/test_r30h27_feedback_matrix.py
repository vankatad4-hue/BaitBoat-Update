import importlib.util
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location(
 "r30h27",ROOT/"Boat/tools/verify_r30h27_feedback_matrix.py")
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
class TestR30H27Provenance(unittest.TestCase):
 def rd(self,path):
  return (ROOT/path).read_text(encoding="utf-8")
 def changed(self,path,needle,replacement):
  original=self.rd(path)
  self.assertIn(needle,original)
  modified=original.replace(needle,replacement,1)
  errors=module.audit(overrides={path:modified})
  self.assertTrue(errors, "unsafe provenance mutation was not rejected")
 def test_all_unsigned_evidence_and_gates(self):
  self.assertEqual([],module.audit())
 def test_cannot_claim_independent_signal_without_sensor(self):
  self.changed(module.MATRIX,",UNVERIFIED,NOT_RUN,",",SIGNED,PASS,")
 def test_cannot_fake_physical_witness(self):
  self.changed(module.MATRIX,",UNVERIFIED,NOT_RUN,"," ,SIGNED,PASS,")
 def test_cannot_claim_commissioned_source(self):
  self.changed(module.MATRIX,",UNSELECTED,UNASSIGNED,",",COMMISSIONED,GPIO1,")
 def test_cannot_change_source_guard_to_commissioned(self):
  self.changed("Boat/source/boat_feedback_provenance_audit.h",
       "PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE = false",
       "PHYSICAL_FEEDBACK_SIGNOFF_COMPLETE = true")
 def test_cannot_attach_unsupported_hardware_driver(self):
  self.changed("Boat/source/firmware/Flytec_Boat.ino",
       "application.tick();","application.attach(unsafe_hal);application.tick();")
 def test_mcp_output_readback_not_signed(self):
  self.changed(module.MATRIX,",NOT_INDEPENDENT,NOT_RUN,",",SIGNED,PASS,")
if __name__=="__main__":
 unittest.main()
