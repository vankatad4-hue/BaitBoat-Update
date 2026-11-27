#!/usr/bin/env python3
"""Convert a legally obtained MBTiles archive into an importable Flytec map pack.

This tool reads a local archive only. It does not download imagery and must not
be used with sources whose terms prohibit offline storage, conversion or display.
"""
import argparse
import sqlite3
from io import BytesIO
from pathlib import Path

from PIL import Image
from mbtiles_area_export import (
    MANIFEST_LIMIT,
    mbtiles_tms_to_xyz_y,
    rgb565_le,
    single_line,
    validate_metadata,
)
from map_area_planner import MAX_PACK_TILES


def export_archive(mbtiles_path, output_dir, style, attribution, licence_url):
    """Convert every tile in a local MBTiles archive to firmware pack format."""
    attribution = single_line(attribution)
    licence_url = single_line(licence_url)
    db = sqlite3.connect(f"file:{Path(mbtiles_path).resolve()}?mode=ro", uri=True)
    try:
        metadata = dict(db.execute("SELECT name,value FROM metadata"))
        provider = single_line(metadata.get("name", Path(mbtiles_path).name))
        validate_metadata(style, provider, attribution, licence_url)
        tile_count = int(db.execute("SELECT COUNT(*) FROM tiles").fetchone()[0])
        if not tile_count or tile_count > MAX_PACK_TILES:
            raise ValueError(f"Archive must contain 1..{MAX_PACK_TILES} tiles to fit the firmware pack limit")

        maps = Path(output_dir) / "maps"
        tile_root = maps / style
        tile_root.mkdir(parents=True, exist_ok=True)
        manifest_path = maps / "manifest.txt"
        # Invalidate an old pack before touching its tile files. A failed export
        # must not leave a stale completion marker over a partially replaced pack.
        if manifest_path.exists():
            manifest_path.unlink()

        index_rows = []
        zooms = set()
        previous = None
        rows = db.execute(
            "SELECT zoom_level,tile_column,tile_row,tile_data "
            "FROM tiles ORDER BY zoom_level,tile_column,tile_row"
        )
        for z_value, x_value, tms_y_value, data in rows:
            z, x, tms_y = int(z_value), int(x_value), int(tms_y_value)
            if not 0 <= z <= 22:
                raise ValueError(f"Unsupported tile zoom: {z}")
            n = 1 << z
            y = mbtiles_tms_to_xyz_y(tms_y, z)
            if not (0 <= x < n and 0 <= y < n):
                raise ValueError(f"Tile coordinate out of range z={z} x={x} y={y}")
            key = (z, x, y)
            if key == previous:
                raise ValueError(f"Duplicate tile z={z} x={x} y={y}")
            previous = key
            try:
                image = Image.open(BytesIO(data))
            except Exception as exc:
                raise RuntimeError(f"Unreadable tile z={z} x={x} y={y}") from exc
            destination = tile_root / str(z) / str(x) / f"{y}.rgb565"
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(rgb565_le(image))
            index_rows.append((z, x, y))
            zooms.add(z)

        if len(index_rows) != tile_count:
            raise ValueError("MBTiles row count changed during export")
        index_rows.sort()
        index_path = maps / "coverage_index.csv"
        index_path.write_text(
            "z,x,y,present\n" + "".join(f"{z},{x},{y},1\n" for z, x, y in index_rows),
            encoding="utf-8",
        )
        bounds = single_line(metadata.get("bounds", "unspecified"))
        manifest = [
            "FLYTEC_MAP1",
            "format=raw-rgb565-le",
            "tile_size=256",
            "scheme=XYZ-WebMercator",
            f"style={style}",
            f"name={provider}",
            f"bounds={bounds}",
            f"zoom_levels={','.join(map(str, sorted(zooms)))}",
            f"planned_tile_count={tile_count}",
            f"exported_tile_count={tile_count}",
            "missing_tile_count=0",
            "coverage_index=coverage_index.csv",
            "pack_complete=true",
            f"provider={provider}",
            f"attribution={attribution}",
            f"licence_url={licence_url}",
        ]
        manifest_text = "\n".join(manifest) + "\n"
        if len(manifest_text.encode("utf-8")) >= MANIFEST_LIMIT:
            raise ValueError("Generated manifest exceeds the firmware validation limit")
        manifest_path.write_text(manifest_text, encoding="utf-8")
        return tile_count
    finally:
        db.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mbtiles", type=Path, help="Input MBTiles database")
    parser.add_argument("output", type=Path, help="Output folder; contains maps/")
    parser.add_argument("--style", choices=("street", "satellite"), required=True)
    parser.add_argument("--attribution", required=True, help="Required provider attribution text")
    parser.add_argument("--licence-url", required=True, help="Source licence or terms URL")
    args = parser.parse_args()
    count = export_archive(args.mbtiles, args.output, args.style, args.attribution, args.licence_url)
    print(f"Created importable {count}-tile pack under {args.output / 'maps'}")


if __name__ == "__main__":
    main()
