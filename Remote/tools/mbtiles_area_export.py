#!/usr/bin/env python3
"""Export planned XYZ tiles from an MBTiles archive to an offline Flytec pack. No downloads."""
import argparse, json, sqlite3
from io import BytesIO
from pathlib import Path
from PIL import Image
from map_area_planner import build_plan

def mbtiles_tms_to_xyz_y(tms_y, zoom):
    return (1 << zoom) - 1 - tms_y

def rgb565_le(image):
    image=image.convert("RGB")
    if image.size!=(256,256):
        image=image.resize((256,256),Image.Resampling.LANCZOS)
    out=bytearray(256*256*2)
    for i,(r,g,b) in enumerate(image.getdata()):
        v=((r>>3)<<11)|((g>>2)<<5)|(b>>3)
        out[i*2]=v&0xff;out[i*2+1]=(v>>8)&0xff
    return out

def single_line(value):
    return str(value).replace("\r"," ").replace("\n"," ").strip()

def export_area(mbtiles_path,output_dir,style,lat,lon,side_m,zooms,provider,attribution,licence_url):
    provider=single_line(provider);attribution=single_line(attribution);licence_url=single_line(licence_url)
    if not zooms:raise ValueError("At least one zoom level is required")
    plan=build_plan(lat,lon,side_m,zooms,provider,attribution,licence_url)
    wanted={(t["z"],t["x"],t["y"]) for t in plan["tiles"]}
    root=Path(output_dir);maps=root/"maps";tile_root=maps/style;tile_root.mkdir(parents=True,exist_ok=True)
    db=sqlite3.connect(f"file:{Path(mbtiles_path).resolve()}?mode=ro",uri=True)
    metadata=dict(db.execute("SELECT name,value FROM metadata"));exported=0;missing=[];index_rows=[]
    try:
        for z,x,y in sorted(wanted):
            tms_y=mbtiles_tms_to_xyz_y(y,z)
            row=db.execute("SELECT tile_data FROM tiles WHERE zoom_level=? AND tile_column=? AND tile_row=?",(z,x,tms_y)).fetchone()
            if row is None:
                missing.append({"z":z,"x":x,"y":y});index_rows.append((z,x,y,0));continue
            try:image=Image.open(BytesIO(row[0]))
            except Exception as exc:raise RuntimeError(f"Bad tile z={z} x={x} y={y}") from exc
            dest=tile_root/str(z)/str(x)/f"{y}.rgb565";dest.parent.mkdir(parents=True,exist_ok=True)
            dest.write_bytes(rgb565_le(image));exported+=1;index_rows.append((z,x,y,1))
    finally:db.close()
    plan.update(exported_tile_count=exported,missing_tile_count=len(missing),missing_tiles=missing,
                pack_complete=(not missing),mbtiles_name=single_line(metadata.get("name",Path(mbtiles_path).name)),
                mbtiles_bounds=single_line(metadata.get("bounds","unspecified")),style=style)
    (maps/"coverage_index.csv").write_text("z,x,y,present\n"+"".join(f"{z},{x},{y},{p}\n" for z,x,y,p in sorted(index_rows)),encoding="utf-8")
    (maps/"area_plan.json").write_text(json.dumps(plan,indent=2)+"\n",encoding="utf-8")
    manifest=["FLYTEC_MAP1","format=raw-rgb565-le","tile_size=256","scheme=XYZ-WebMercator",
        f"style={style}",f"name={plan['mbtiles_name']}",f"bounds={plan['bounds_wgs84']}",
        f"zoom_levels={','.join(map(str,plan['zooms']))}",f"planned_tile_count={len(wanted)}",
        f"exported_tile_count={exported}",f"missing_tile_count={len(missing)}","coverage_index=coverage_index.csv",
        f"pack_complete={'true' if not missing else 'false'}",f"provider={provider}",
        f"attribution={attribution}",f"licence_url={licence_url}"]
    (maps/"manifest.txt").write_text("\n".join(manifest)+"\n",encoding="utf-8")
    return plan

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("mbtiles",type=Path);ap.add_argument("output",type=Path)
    ap.add_argument("--style",choices=("street","satellite"),required=True)
    ap.add_argument("--lat",type=float,required=True);ap.add_argument("--lon",type=float,required=True)
    ap.add_argument("--side-m",type=float,default=1000);ap.add_argument("--zooms",default="14,15,16,17")
    ap.add_argument("--provider",required=True);ap.add_argument("--attribution",required=True);ap.add_argument("--licence-url",required=True)
    a=ap.parse_args();zooms=[int(v.strip()) for v in a.zooms.split(",") if v.strip()]
    p=export_area(a.mbtiles,a.output,a.style,a.lat,a.lon,a.side_m,zooms,a.provider,a.attribution,a.licence_url)
    print(f"Exported {p['exported_tile_count']}/{p['unique_tile_count']} tiles; complete={p['pack_complete']}")
if __name__=="__main__":main()
