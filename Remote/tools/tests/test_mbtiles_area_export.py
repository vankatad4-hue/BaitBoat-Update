import unittest
import sys, sqlite3, tempfile, csv
from pathlib import Path
from io import BytesIO
sys.path.insert(0, str(Path(__file__).resolve().parents[18]))
from mbtiles_area_export import mbtiles_tms_to_xyz_y, rgb565_le, export_area
from map_area_planner import build_plan
from PIL import Image

class MBTilesAreaExportTests(unittest.TestCase):
    def test_tms_to_xyz_flip(self):
        self.assertEqual(mbtiles_tms_to_xyz_y(0,3),7)
        self.assertEqual(mbtiles_tms_to_xyz_y(7,3),0)
    def test_rgb565_tile_size_and_red_pixel(self):
        image=Image.new("RGB",(256,256),(255,0,0))
        data=rgb565_le(image)
        self.assertEqual(len(data),131072)
        self.assertEqual(data[:2],bytes((0x00,0xF8)))
    def test_coverage_index_records_present_and_missing(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);dbpath=root/"fixture.mbtiles"
            db=sqlite3.connect(dbpath)
            db.execute("CREATE TABLE metadata(name TEXT,value TEXT)")
            db.executemany("INSERT INTO metadata VALUES (?,?)",[("name","fixture"),("bounds","-180,-85,180,85")])
            db.execute("CREATE TABLE tiles(zoom_level INTEGER,tile_column INTEGER,tile_row INTEGER,tile_data BLOB)")
            planned=build_plan(43,25,1000,[1],"fixture","attrib","https://example.test/licence")
            z,x,y=planned["tiles"][0]["z"],planned["tiles"][0]["x"],planned["tiles"][0]["y"]
            image=Image.new("RGB",(256,256),(10,20,30));buffer=BytesIO();image.save(buffer,format="PNG")
            db.execute("INSERT INTO tiles VALUES (?,?,?,?)",(z,x,(1<<z)-1-y,buffer.getvalue()))
            db.commit();db.close()
            plan=export_area(dbpath,root/"out","street",43,25,1000,[1],"fixture","attrib","https://example.test/licence")
            idx=root/"out"/"maps"/"coverage_index.csv"
            with idx.open(newline="") as f:rows=list(csv.DictReader(f))
            self.assertEqual(len(rows),plan["unique_tile_count"])
            self.assertEqual(sum(row["present"]=="1" for row in rows),1)
            self.assertEqual(sum(row["present"]=="0" for row in rows),len(rows)-1)
            self.assertFalse(plan["pack_complete"])
            self.assertTrue((root/"out"/"maps"/"manifest.txt").exists())
if __name__=="__main__":unittest.main()
