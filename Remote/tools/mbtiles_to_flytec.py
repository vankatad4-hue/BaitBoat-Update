#!/usr/bin/env python3
"""Convert a legally obtained MBTiles archive into Flytec RGB565 SD map tiles.
The source must explicitly permit offline storage, conversion and display.
This script does not download imagery and must not be used with Google tiles.
"""
import argparse
import sqlite3
from pathlib import Path
from PIL import Image
from io import BytesIO

def rgb565_bytes(image):
    image=image.convert("RGB")
    if image.size!=(256,256):
        image=image.resize((256,256),Image.Resampling.LANCZOS)
    out=bytearray(256*256*2)
    for i,(r,g,b) in enumerate(image.getdata()):
        v=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
        out[i*2]=v&255;out[i*2+1]=(v>>8)&255
    return out

def single_line(value):
    return str(value).replace("\r"," ").replace("\n"," ").strip()

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("mbtiles",type=Path,help="Input MBTiles database")
    ap.add_argument("output",type=Path,help="Output folder (contains maps subfolder)")
    ap.add_argument("--style",choices=("street","satellite"),required=True)
    ap.add_argument("--attribution",required=True,help="Required provider attribution text")
    ap.add_argument("--licence-url",required=True,help="Source licence or terms URL")
    args=ap.parse_args()
    attribution=single_line(args.attribution);licence=single_line(args.licence_url)
    db=sqlite3.connect(f"file:{args.mbtiles.resolve()}?mode=ro",uri=True)
    metadata=dict(db.execute("SELECT name,value FROM metadata"))
    rows=db.execute("SELECT zoom_level,tile_column,tile_row,tile_data FROM tiles ORDER BY zoom_level,tile_column,tile_row")
    count=0;zooms=set();root=args.output/"maps"/args.style
    for z,x,tms_y,data in rows:
        y=(1<<z)-1-tms_y
        try: image=Image.open(BytesIO(data))
        except Exception as exc: raise RuntimeError(f"Unreadable tile z={z} x={x} y={y}") from exc
        dest=root/str(z)/str(x)/f"{y}.rgb565";dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_bytes(rgb565_bytes(image));count+=1;zooms.add(z)
    db.close()
    name=single_line(metadata.get("name",args.mbtiles.name))
    bounds=single_line(metadata.get("bounds","unknown"))
    manifest=["FLYTEC_MAP1","format=raw-rgb565-le","tile_size=256","scheme=XYZ-WebMercator",
        f"style={args.style}",f"name={name}",f"bounds={bounds}",
        f"zoom_levels={','.join(map(str,sorted(zooms)))}",f"tile_count={count}",
        "pack_complete=true",f"attribution={attribution}",f"licence_url={licence}"]
    m=args.output/"maps"/"manifest.txt";m.parent.mkdir(parents=True,exist_ok=True)
    m.write_text("\n".join(manifest)+"\n",encoding="utf-8")
    print(f"Created {count} tiles; zoom levels {sorted(zooms)}; complete pack manifest: {m}")
if __name__=="__main__":main()
