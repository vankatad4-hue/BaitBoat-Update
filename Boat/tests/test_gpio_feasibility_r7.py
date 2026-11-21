import pathlib
import sys
import unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
import gpio_feasibility_r7 as g

class BudgetTests(unittest.TestCase):
    def test_new_onboard_gpio_budget(self):
        self.assertEqual(len(g.CONSUMERS),10)
        self.assertEqual(len(g.CANDIDATES),11)
        self.assertEqual(g.budget()["shortfall"],0)
        self.assertEqual(g.budget(external_e22=True)["shortfall"],1)
        self.assertEqual(g.budget(g.STRICT_CANDIDATES)["shortfall"],5)
    def test_expansion_can_only_be_conditional(self):
        self.assertTrue(g.budget()["feasible_on_count_alone"])
        self.assertFalse(g.budget(external_e22=True)["feasible_on_count_alone"])
        self.assertFalse(g.validate_assignment({},g.CANDIDATES))
        self.assertTrue(g.REQUIRED_NATIVE.issubset(g.CONSUMERS))
        self.assertTrue(g.REQUIRED_NATIVE.isdisjoint(g.EXPANDER_ELIGIBLE))
        self.assertTrue({"mdd_dir_left","mdd_dir_right"}.issubset(g.REQUIRED_NATIVE))
    def test_signed_and_collision_guards(self):
        assignment={name:pin for name,pin in zip(g.CONSUMERS,sorted(g.CANDIDATES))}
        self.assertTrue(g.validate_assignment(assignment,g.CANDIDATES,True))
        self.assertFalse(g.validate_assignment(assignment,g.CANDIDATES,False))
        assignment["independent_gate_permit"]=assignment["nrf_ce"]
        self.assertFalse(g.validate_assignment(assignment,g.CANDIDATES,True))
    def test_matrix_is_not_verified(self):
        rows=g.load_rows(ROOT/"docs"/"BOAT_GPIO_CANDIDATE_MATRIX_R7.csv")
        self.assertEqual({int(row["GPIO"]) for row in rows},g.CANDIDATES)
        self.assertTrue(all("candidate" in row["Potential use"].lower()
                            or "bus only" in row["Potential use"].lower()
                            for row in rows))
if __name__=="__main__":
    unittest.main()
