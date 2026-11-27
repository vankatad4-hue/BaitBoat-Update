#!/usr/bin/env python3
"""Plan an offline map square and estimate Flytec RGB565 tile storage.

This tool calculates geographic/tile coverage only. It does not fetch map imagery.
Only use a map provider/data source whose licence permits offline downloading,
storage, conversion and display. Never use it to download Google map tiles.
"""
import argparse
import json
import math
from pathlib import Path

TILE_SIZE = 256
BYTES_PER_TILE = TILE_SIZE * TILE_SIZE * 2
MAX_PACK_TILES = 6000  # Match the firmware coverage-index and importer limits.
MERCATOR_LIMIT = 85.05112878
METERS_PER_DEG_LAT = 111_320.0


def square_bounds(center_lat, center_lon, side_m=1000.0):
    if not all(math.isfinite(value) for value in (center_lat, center_lon, side_m)):
        raise ValueError("latitude, longitude and side length must be finite")
    if not (-90 <= center_lat <= 90):
        raise ValueError("latitude must be between -90 and 90")
    if not (-180 <= center_lon <= 180):
        raise ValueError("longitude must be between -180 and 180")
    if side_m <= 0:
        raise ValueError("side length must be positive")
    half = side_m / 2.0
    lat_delta = half / METERS_PER_DEG_LAT
    cos_lat = math.cos(math.radians(center_lat))
    if abs(cos_lat) < 1e-6:
        raise ValueError("area too close to a pole for this square approximation")
    lon_delta = half / (METERS_PER_DEG_LAT * cos_lat)
    south = max(-MERCATOR_LIMIT, center_lat - lat_delta)
    north = min(MERCATOR_LIMIT, center_lat + lat_delta)
    west = center_lon - lon_delta
    east = center_lon + lon_delta
    # Keep bounds as an unwrapped eastward interval; wrap only tile indices.
    return {"south": south, "north": north, "west": west, "east": east}


def lon_to_tile_x(lon, zoom):
    n = 1 << zoom
    return math.floor((lon + 180.0) / 360.0 * n) % n


def lat_to_tile_y(lat, zoom):
    n = 1 << zoom
    lat = max(-MERCATOR_LIMIT, min(MERCATOR_LIMIT, lat))
    rad = math.radians(lat)
    value = (1.0 - math.asinh(math.tan(rad)) / math.pi) / 2.0 * n
    return max(0, min(n - 1, math.floor(value)))


def tiles_for_bounds(bounds, zoom):
    if not (0 <= zoom <= 22):
        raise ValueError("zoom must be 0..22")
    if not all(math.isfinite(bounds[key]) for key in ("west", "east", "south", "north")):
        raise ValueError("tile bounds must be finite")
    n = 1 << zoom
    x0 = math.floor((bounds["west"] + 180.0) / 360.0 * n)
    x1 = math.floor((bounds["east"] + 180.0) / 360.0 * n)
    y0 = lat_to_tile_y(bounds["north"], zoom)
    y1 = lat_to_tile_y(bounds["south"], zoom)
    y_start, y_end = min(y0, y1), max(y0, y1)
    y_count = y_end - y_start + 1
    # Count unique wrapped x tiles before building a range, including areas
    # wider than a world, so huge requests fail without allocating huge lists.
    x_count = min(n, x1 - x0 + 1)
    if x_count * y_count > MAX_PACK_TILES:
        raise ValueError(f"zoom {zoom} exceeds the {MAX_PACK_TILES}-tile firmware pack limit")
    ys = range(y_start, y_end + 1)
    xs = sorted({(x0 + offset) % n for offset in range(x_count)})
    return [(x, y) for x in xs for y in ys]


def build_plan(lat, lon, side_m, zooms, source_name, attribution, licence_url):
    if not zooms:
        raise ValueError("at least one zoom level is required")
    bounds = square_bounds(lat, lon, side_m)
    per_zoom = {}
    tile_set = set()
    for z in sorted(set(zooms)):
        coords = tiles_for_bounds(bounds, z)
        if len(tile_set) + len(coords) > MAX_PACK_TILES:
            raise ValueError(f"plan exceeds the {MAX_PACK_TILES}-tile firmware pack limit")
        per_zoom[str(z)] = len(coords)
        tile_set.update((z, x, y) for x, y in coords)
    total = len(tile_set)
    raw_bytes = total * BYTES_PER_TILE
    return {
        "schema": "flytec-map-area-plan-v1",
        "status": "PLAN_ONLY_NOT_DOWNLOADED",
        "centre": {"latitude": lat, "longitude": lon},
        "side_m": side_m,
        "bounds_unwrapped": bounds,
        "bounds_wgs84": {
            "south": bounds["south"], "north": bounds["north"],
            "west": ((bounds["west"] + 180) % 360) - 180,
            "east": ((bounds["east"] + 180) % 360) - 180,
            "crosses_dateline": bounds["west"] < -180 or bounds["east"] > 180,
        },
        "zooms": sorted(set(zooms)),
        "tile_counts_by_zoom": per_zoom,
        "unique_tile_count": total,
        "bytes_per_tile_rgb565": BYTES_PER_TILE,
        "raw_storage_bytes": raw_bytes,
        "recommended_free_bytes_with_10pct_margin": math.ceil(raw_bytes * 1.10),
        "provider": source_name,
        "attribution": attribution,
        "licence_url": licence_url,
        "tiles": [{"z": z, "x": x, "y": y} for z, x, y in sorted(tile_set)],
        "notice": "Tile coverage extends to tile boundaries. Verify provider licence and attribution. This planner never downloads imagery.",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lat", type=float, required=True, help="Centre latitude")
    parser.add_argument("--lon", type=float, required=True, help="Centre longitude")
    parser.add_argument("--side-m", type=float, default=1000, help="Square side length in metres (default 1000)")
    parser.add_argument("--zooms", default="14,15,16,17", help="Comma-separated zoom levels")
    parser.add_argument("--source", required=True, help="Offline source/provider name")
    parser.add_argument("--attribution", required=True, help="Attribution required by source licence")
    parser.add_argument("--licence-url", required=True, help="URL to source licence/terms")
    parser.add_argument("--output", type=Path, help="Optional JSON plan output file")
    args = parser.parse_args()
    zooms = [int(value) for value in args.zooms.split(",") if value.strip()]
    plan = build_plan(args.lat, args.lon, args.side_m, zooms, args.source, args.attribution, args.licence_url)
    rendered = json.dumps(plan, indent=2)
    if args.output:
        args.output.write_text(rendered + "\n", encoding="utf-8")
    print(rendered)


if __name__ == "__main__":
    main()
