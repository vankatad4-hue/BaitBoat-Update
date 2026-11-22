"""Negative mutation tests for R30A design handoff; no hardware exercised."""
import copy
import importlib.util
from pathlib import Path
import unittest

PATH=Path(__file__).resolve().parents[1]/"tools"/"verify_r30a_assembly_package.py"
spec=importlib.util.spec_from_file_location("r30a", PATH)
module=importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

class EffectiveNetlistTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.r23=module.rows("Boat/docs/BOAT_R23_MODULE_CONNECTION_MATRIX.csv")
        cls.r27=module.rows("Boat/docs/BOAT_R27_INA231_HARNESS_OVERRIDE.csv")
        cls.r29=module.rows("Boat/docs/BOAT_R29_SHARED_3S_BUS_CONNECTIONS.csv")
        cls.effective=module.rows("Boat/docs/BOAT_R30A_EFFECTIVE_ASSEMBLY_NETS.csv")
        cls.gates=module.rows("Boat/docs/BOAT_R30A_READINESS_GATES.csv")
    def valid(self, effective):
        return module.check_netlist(self.r23,self.r27,self.r29,effective)
    def test_exact_architecture_and_undocumented_parts(self):
        self.assertTrue(self.valid(copy.deepcopy(self.effective)))
        self.assertTrue(module.check_readiness(copy.deepcopy(self.gates)))
    def test_reject_retired_ads(self):
        mod=copy.deepcopy(self.effective)
        mod[0]["Net"]="ADS_SCL"
        with self.assertRaises(AssertionError):self.valid(mod)
    def test_reject_separate_actuator_battery(self):
        mod=copy.deepcopy(self.effective)
        next(r for r in mod if r["Net"]=="EKM_FET_VH")["Source"]="SECOND_3S_PACK"
        with self.assertRaises(AssertionError):self.valid(mod)
    def test_reject_shunt_power_wiring(self):
        mod=copy.deepcopy(self.effective)
        next(r for r in mod if r["Net"]=="INA231_SHUNT_PLUS")["Install_Status"]="UNWIRED_PENDING"
        with self.assertRaises(AssertionError):self.valid(mod)
    def test_reject_duplicate_alias_conductor(self):
        mod=copy.deepcopy(self.effective)
        next(r for r in mod if r["Net"]=="INA231_BUS_SENSE")["Install_Status"]="UNWIRED_PENDING"
        with self.assertRaises(AssertionError):self.valid(mod)
    def test_reject_forged_physical_acceptance(self):
        mod=copy.deepcopy(self.gates)
        next(r for r in mod if r["Gate_ID"]=="G13")["State"]="PASS"
        with self.assertRaises(AssertionError):module.check_readiness(mod)
    def test_reject_unqualified_high_current_removal(self):
        mod=copy.deepcopy(self.gates)
        mod=[r for r in mod if r["Gate_ID"]!="G11"]
        with self.assertRaises(AssertionError):module.check_readiness(mod)

if __name__=="__main__":
    unittest.main()
