import numpy as np
from pathlib import Path

path_to_grids = Path("C:/Users/berg.ZALF-AD/Nextcloud/DDR-grids")

elements = ["az", "corine", "dgm", "hft", "hoehe", "klz", "kwb", "nft", "sft", "slope", "steino", "stt"]
bl_data = {
    #"sachsen-anhalt": {"id": 6, "xmin": 5193000, "xmax": 5377000, "ymin": 5647000, "ymax": 5885000},
    #"mecklenburg-vorpommern": {"id": 2, "xmin": 5206000, "xmax": 5462000, "ymin": 5891000, "ymax": 6064000},
    "sachsen": {"id": 10, "xmin": 5278000, "xmax": 5505000, "ymin": 5561000, "ymax": 5731000},
    "thueringen": {"id": 9, "xmin": 5137000, "xmax": 5336000, "ymin": 5571000, "ymax": 5733000},
    "brandenburg": {"id": 5, "xmin": 5249000, "xmax": 5486000, "ymin": 5691000, "ymax": 5938000}}

header = """ncols         {}
nrows         {}
xllcorner     {}
yllcorner     {}
cellsize      100
NODATA_value  -9999"""

elem_grids = {}
for elem in elements:
    grid = np.loadtxt(path_to_grids / "{}_ddr_100_gk5.asc".format(elem), dtype=int, skiprows=6)
    if grid.shape[1] < 3651:
        print(elem, "has just", grid.shape[1], "cols")
    print("loaded elem grid:", elem)
    elem_grids[elem] = grid

bl_grid = np.loadtxt(path_to_grids / "bundeslaender_ddr_100_gk5.asc", dtype=int, skiprows=6)
print("loaded BL grid")

bl_xul = 5138000
bl_yul = 5563000 + 5000 * 100

for bl, data in bl_data.items():
    print("BL:", bl)

    target_nrows = int((data["ymax"] - data["ymin"]) / 100.0)
    target_ncols = int((data["xmax"] - data["xmin"]) / 100.0)

    target_grids = {}
    for elem in elements:
        target_grids[elem] = np.full((target_nrows, target_ncols), -9999, dtype=int)

    nrows, ncols = bl_grid.shape
    target_row = 0
    for row in range(nrows):
        print(row, end=" ", flush=True)

        cur_y = bl_yul - row * 100
        if cur_y > data["ymax"] or cur_y < data["ymin"]:
            continue

        target_col = 0
        for col in range(ncols):
            
            cur_x = bl_xul + col * 100

            if cur_x < data["xmin"] or cur_x > data["xmax"]:
                continue

            bl_id = bl_grid[row, col]

            if bl_id == data["id"]:
                
                for target_name, target_grid in target_grids.items():
                    target_grid[target_row, target_col] = elem_grids[target_name][row, col] 

            target_col += 1

        target_row += 1

    print("writing grids for BL:", bl)
    for target_name, target_grid in target_grids.items():
        np.savetxt(path_to_grids / bl / "{}_{}_gk5.asc".format(target_name, bl), 
        target_grid, header=header.format(target_ncols, target_nrows, data["xmin"], data["ymin"]), fmt="%d", comments="") 
        print("wrote elem grid:", target_name)


