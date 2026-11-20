#!/usr/bin/env python3
"""Convert a legally obtained MBTiles archive into Flytec's raw RGB565 tile pack."""
import argparse, sqlite3
from pathlib import Path
from PIL import Image
from io import BytesIO

def rgb565_bytes(image):
    image=image.convert("RGB")
    if image.size!=(256,256):
        image=image.resize((256,256),Image.Resampling.LANCZOS)
    out=bytearray(256*256*2)
    j=0
    for r,g,b in image.getdata():
        v=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
        out[j]=v&255;out[j+1]=(v>>8)&255;j+=2
    return out

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("mbtiles",type=Path,help="Input MBTiles SQLite database")
    ap.add_argument("output",type=Path,help="Output folder (contains maps subfolder)")
    ap.add_argument("--style",choices=("street","satellite"),required=True)
    ap.add_argument("--attribution",required=True,help="Required source/provider attribution text")
    args=ap.parse_args()
    db=sqlite3.connect(f"file:{args.mbtiles.resolve()}?mode=ro",uri=True)
    metadata=dict(db.execute("SELECT name,value FROM metadata"))
    tiles=db.execute("SELECT zoom_level,tile_column,tile_row,tile_data FROM tiles ORDER BY zoom_level,tile_column,tile_row")
    count=0;zooms=set()
    root=args.output/"maps"/args.style
    for z,x,tms_y,data in tiles:
        y=(1<<z)-1-tms_y
        try: im=Image.open(BytesIO(data))
        except Exception as exc: raise RuntimeError(f"Unreadable tile z={z} x={x} y={y}") from exc
        dest=root/str(z)/str(x)/f"{y}.rgb565";dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_bytes(rgb565_bytes(im));count+=1;zooms.add(z)
    db.close()
    bounds=metadata.get("bounds","unknown")
    name=metadata.get("name",args.mbtiles.name)
    manifest=["FLYTEC_MAP1","format=raw-rgb565-le","tile_size=256","scheme=XYZ-WebMercator",f"style={args.style}",f"name={name}",f"bounds={bounds}",f"zoom_levels={','.join(map(str,sorted(zooms)))}",f"tile_count={count}",f"attribution={args.attribution}"]
    m=args.output/"maps"/"manifest.txt";m.parent.mkdir(parents=True,exist_ok=True);m.write_text("\n".join(manifest)+"\n",encoding="utf-8")
    print(f"Created {count} tiles; zoom levels {sorted(zooms)}; manifest: {m}")
if __name__=="__main__":main()
