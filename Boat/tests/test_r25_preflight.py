"""Host-only safety evidence gate regressions (all synthetic fixtures)."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

SOURCE=Path(__file__).resolve().parents[1]/"tools/verify_r25_preflight.py"
spec=importlib.util.spec_from_file_location("r25",SOURCE)
r25=importlib.util.module_from_spec(spec)
spec.loader.exec_module(r25)

class ElectricalEvidenceTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root=Path(self.tmp.name)
        self.rows=[dict.fromkeys(r25.COLUMNS,"") for _ in r25.IDS]
        for row,check_id in zip(self.rows,r25.IDS):
            row["Check_ID"]=check_id
            row["Status"]="NOT_RUN"
    def sign(self,name,measured=None,status="PASS"):
        row=next(r for r in self.rows if r["Check_ID"]==name)
        row.update(Status=status,Board_Revision="WROOM-2 synthetic fixture",
                   Operator="test fixture",Date="2026-11-22",
                   Evidence_Path="Boat/evidence/r25/"+name+".csv")
        if measured is not None: row["Measured_V"]=str(measured)
        path=self.root/row["Evidence_Path"]
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text("SYNTHETIC TEST ONLY",encoding="utf-8")
        return row
    def assertBlocked(self):
        self.assertTrue(r25.validate(self.rows,self.root))
    def test_blank_remains_unsigned(self):
        self.assertEqual(r25.validate(self.rows,self.root,require_blank=True),[])
        self.assertFalse(any(r["Status"]=="PASS" for r in self.rows))
    def test_wrong_ids_rejected(self):
        self.rows.pop(); self.assertBlocked()
        self.rows.append(dict(self.rows[0])); self.assertBlocked()
    def test_m100_5v_output_rejected(self):
        self.sign("M100_TX_HIGH_ISOLATED",5.0); self.assertBlocked()
    def test_e22_5v_output_rejected(self):
        self.sign("E22_AUX_HIGH_ISOLATED",5.0); self.assertBlocked()
    def test_digital_high_too_low_rejected(self):
        self.sign("M100_TX_HIGH_ISOLATED",1.1); self.assertBlocked()
    def test_adc_unpowered_delta_rejected(self):
        self.sign("ADC_POWERDOWN_PEAK_AIN_MINUS_VDD",2.272); self.assertBlocked()
    def test_safe_negative_adc_delta_screen(self):
        self.sign("ADC_POWERUP_PEAK_AIN_MINUS_VDD",-1.028)
        self.assertEqual(r25.validate(self.rows,self.root),[])
    def test_missing_evidence_rejected(self):
        row=self.sign("MAIN_5V_LOAD",5.0)
        (self.root/row["Evidence_Path"]).unlink()
        self.assertBlocked()
    def test_pwm_boot_high_rejected(self):
        self.sign("PWM39_BOOT_MAX_V",2.5); self.assertBlocked()
    def test_manual_cannot_invent_voltage(self):
        self.sign("INDEPENDENT_STOP_CUTOFF_ACCEPTED",3.3); self.assertBlocked()
    def test_ci_baseline_rejects_signoff(self):
        self.sign("MAIN_5V_LOAD",5.0)
        self.assertTrue(r25.validate(self.rows,self.root,require_blank=True))
    def test_valid_numeric_screen_is_not_full_acceptance(self):
        self.sign("MAIN_5V_LOAD",5.0)
        self.assertEqual(r25.validate(self.rows,self.root),[])
        self.assertEqual(sum(r["Status"]=="PASS" for r in self.rows),1)
    def test_failed_measurement_is_blocker(self):
        self.sign("M100_TX_HIGH_ISOLATED",2.8,status="FAIL"); self.assertBlocked()

if __name__=="__main__": unittest.main()
