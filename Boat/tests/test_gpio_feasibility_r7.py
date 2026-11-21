import pathlib
import sys
import unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"tools"))
import gpio_feasibility_r7 as g

class BudgetTests(unittest.TestCase):
    def test_no_imaginary_12_pin_solution(self):
        self.assertEqual(len(g.CONSUMERS),12)
        self.assertEqual(len(g.CANDIDATES),11)
        self.assertEqual(g.budget()["shortfall"],1)
        self.assertEqual(g.budget(g.STRICT_CANDIDATES)["shortfall"],7)
    def test_expansion_can_only_be_conditional(self):
        self.assertTrue(g.budget(i2c_uart_bridge=True)["feasible_on_count_alone"])
        self.assertTrue(g.budget(direction_expander=True)["feasible_on_count_alone"])
        self.assertFalse(g.validate_assignment({},g.CANDIDATES))
        self.assertTrue(g.REQUIRED_NATIVE.issubset(g.CONSUMERS))
        self.assertTrue(g.REQUIRED_NATIVE.isdisjoint(g.EXPANDER_ELIGIBLE))
    def test_signed_and_collision_guards(self):
        assignment={name:pin for name,pin in zip(g.CONSUMERS,g.CANDIDATES)}
        self.assertFalse(g.validate_assignment(assignment,g.CANDIDATES,True))
        assignment["independent_gate_permit"]=17
        self.assertFalse(g.validate_assignment(assignment,g.CANDIDATES,True))
    def test_matrix_is_not_verified(self):
        rows=g.load_rows(ROOT/"docs"/"BOAT_GPIO_CANDIDATE_MATRIX_R7.csv")
        self.assertEqual({int(row["GPIO"]) for row in rows},g.CANDIDATES)
        self.assertTrue(all("candidate" in row["Potential use"].lower()
                            or "bus only" in row["Potential use"].lower()
                            for row in rows))
if __name__=="__main__":
    unittest.main()
