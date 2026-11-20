import unittest
from map_area_planner import square_bounds, tiles_for_bounds, build_plan

class AreaPlannerTests(unittest.TestCase):
    def test_one_km_area_latitude_size_approx(self):
        b = square_bounds(43.0, 25.0, 1000)
        self.assertAlmostEqual(b["north"] - b["south"], 1000 / 111320, places=8)

    def test_tile_count_positive_and_deduplicated(self):
        b = square_bounds(43.0, 25.0, 1000)
        coords = tiles_for_bounds(b, 16)
        self.assertGreater(len(coords), 0)
        self.assertEqual(len(coords), len(set(coords)))

    def test_dateline_wrap(self):
        b = square_bounds(0.0, 179.999, 1000)
        coords = tiles_for_bounds(b, 8)
        self.assertTrue(any(x == 0 for x, _ in coords))
        self.assertTrue(any(x == 255 for x, _ in coords))

    def test_storage_estimate(self):
        plan = build_plan(43, 25, 1000, [14, 15], "Example source", "Attribution", "https://example.org/licence")
        self.assertEqual(plan["raw_storage_bytes"], plan["unique_tile_count"] * 131072)
        self.assertGreaterEqual(plan["recommended_free_bytes_with_10pct_margin"], plan["raw_storage_bytes"])

if __name__ == "__main__":
    unittest.main()
