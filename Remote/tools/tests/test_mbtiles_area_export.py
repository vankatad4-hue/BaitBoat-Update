import unittest
import sys, sqlite3, tempfile, csv
from pathlib import Path
from io import BytesIO
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from mbtiles_area_export import mbtiles_tms_to_xyz_y, rgb565_le, export_area
from mbtiles_to_flytec import export_archive
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
            planned=build_plan(43,25,1000,[18],"fixture","attrib","https://example.test/licence")
            z,x,y=planned["tiles"][0]["z"],planned["tiles"][0]["x"],planned["tiles"][0]["y"]
            image=Image.new("RGB",(256,256),(10,20,30));buffer=BytesIO();image.save(buffer,format="PNG")
            db.execute("INSERT INTO tiles VALUES (?,?,?,?)",(z,x,(1<<z)-1-y,buffer.getvalue()))
            db.commit();db.close()
            plan=export_area(dbpath,root/"out","street",43,25,1000,[18],"fixture","attrib","https://example.test/licence")
            idx=root/"out"/"maps"/"coverage_index.csv"
            with idx.open(newline="") as f:rows=list(csv.DictReader(f))
            self.assertEqual(len(rows),plan["unique_tile_count"])
            self.assertEqual(sum(row["present"]=="1" for row in rows),1)
            self.assertEqual(sum(row["present"]=="0" for row in rows),len(rows)-1)
            self.assertFalse(plan["pack_complete"])
            self.assertTrue((root/"out"/"maps"/"manifest.txt").exists())

    def test_full_archive_export_matches_import_contract_and_sorts_xyz_index(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);dbpath=root/"fixture.mbtiles"
            db=sqlite3.connect(dbpath)
            db.execute("CREATE TABLE metadata(name TEXT,value TEXT)")
            db.executemany("INSERT INTO metadata VALUES (?,?)",[("name","fixture"),("bounds","-180,-85,180,85")])
            db.execute("CREATE TABLE tiles(zoom_level INTEGER,tile_column INTEGER,tile_row INTEGER,tile_data BLOB)")
            image=Image.new("RGB",(256,256),(10,20,30));buffer=BytesIO();image.save(buffer,format="PNG")
            # TMS order is reversed relative to the firmware's sorted XYZ index.
            db.executemany("INSERT INTO tiles VALUES (?,?,?,?)",[(2,1,2,buffer.getvalue()),(2,1,1,buffer.getvalue())])
            db.commit();db.close()
            self.assertEqual(export_archive(dbpath,root/"out","street","Attribution","https://example.test/licence"),2)
            maps=root/"out"/"maps"
            fields=dict(line.split("=",1) for line in (maps/"manifest.txt").read_text().splitlines()[1:])
            self.assertEqual(fields["pack_complete"],"true")
            self.assertEqual(fields["planned_tile_count"],"2")
            self.assertEqual(fields["exported_tile_count"],"2")
            self.assertEqual(fields["coverage_index"],"coverage_index.csv")
            self.assertEqual((maps/"coverage_index.csv").read_text(),"z,x,y,present\n2,1,1,1\n2,1,2,1\n")
            self.assertEqual((maps/"street/2/1/1.rgb565").stat().st_size,131072)

    def test_bad_reexport_removes_stale_completion_marker(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);dbpath=root/"bad.mbtiles";maps=root/"out"/"maps"
            maps.mkdir(parents=True);marker=maps/"manifest.txt";marker.write_text("old complete pack")
            db=sqlite3.connect(dbpath)
            db.execute("CREATE TABLE metadata(name TEXT,value TEXT)")
            db.executemany("INSERT INTO metadata VALUES (?,?)",[("name","bad"),("bounds","unknown")])
            db.execute("CREATE TABLE tiles(zoom_level INTEGER,tile_column INTEGER,tile_row INTEGER,tile_data BLOB)")
            planned=build_plan(43,25,1000,[5],"bad","Attribution","https://example.test/licence")
            tile=planned["tiles"][0]
            db.execute("INSERT INTO tiles VALUES (?,?,?,?)",(tile["z"],tile["x"],(1<<tile["z"])-1-tile["y"],b"not an image"))
            db.commit();db.close()
            with self.assertRaises(RuntimeError):
                export_area(dbpath,root/"out","street",43,25,1000,[5],"bad","Attribution","https://example.test/licence")
            self.assertFalse(marker.exists())
if __name__=="__main__":unittest.main()
