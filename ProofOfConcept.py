import pygame
from typing import TextIO, Literal
from os.path import abspath

REDDISH = [255, 128, 128]
GREENISH = [128, 255, 128]
BLUEISH = [128, 128, 255]

DARK_REDDISH = [155, 28, 28]
DARK_GREENISH = [28, 155, 28]
DARK_BLUEISH = [28, 28, 155]

BLACKISH = [50, 50, 50]
WHITISH = [200, 200, 200]

COLOR_PICK_RECT = (1075, 200, 50, 50)
R_RECT_TOPLEFT = (1050, 250)
G_RECT_TOPLEFT = (1050, 300)
B_RECT_TOPLEFT = (1050, 350)

FPS = 30

SCREEN_SIZE = (1200, 800)
GRID_SIZE = (1000, 800)
TILE_SIZE = 50


# ------------------------#
#   File IO of the app   #
# ------------------------#


def ParseWorkFile(fh: TextIO) -> dict[tuple[int, int], list[int]]:
    clicked_tiles = dict()

    while True:
        data_line = fh.readline()
        if not data_line:
            break
        elif data_line[0] == "v":
            _, x, y, r, g, b = data_line.split(" ")
            clicked_tiles[(int(x), int(y))] = [int(r) % 256, int(g) % 256, int(b) % 256]
            # done to skip over other 3 vertices of triangle
            fh.readline()
            fh.readline()
            fh.readline()

    return clicked_tiles


def SaveWorkToFile(fh: TextIO, clicked_tiles: dict[tuple[int, int], list[int]]) -> None:
    fh.seek(0, 0)
    fh.truncate(0)
    for i, tile in enumerate(clicked_tiles.keys()):
        fh.write(
            f"\nv {tile[0]} {tile[1]} {clicked_tiles[tile][0]} {clicked_tiles[tile][1]} {clicked_tiles[tile][2]}\n"
        )
        fh.write(
            f"v {tile[0] + TILE_SIZE} {tile[1]} {clicked_tiles[tile][0]} {clicked_tiles[tile][1]} {clicked_tiles[tile][2]}\n"
        )
        fh.write(
            f"v {tile[0]} {tile[1] + TILE_SIZE} {clicked_tiles[tile][0]} {clicked_tiles[tile][1]} {clicked_tiles[tile][2]}\n"
        )
        fh.write(
            f"v {tile[0] + TILE_SIZE} {tile[1] + TILE_SIZE} {clicked_tiles[tile][0]} {clicked_tiles[tile][1]} {clicked_tiles[tile][2]}\n"
        )
        fh.write(f"i {0 + (i * 4)} {1 + (i * 4)} {2 + (i * 4)}\n")
        fh.write(f"i {1 + (i * 4)} {2 + (i * 4)} {3 + (i * 4)}\n")


# --------------------------#
#   Functions of the  app   #
# --------------------------#


def DrawGrid(
    window: pygame.Surface,
    clicked_tiles: dict[tuple[int, int], list[int]],
    move_offset: list[int],
) -> None:
    for row in range(0, GRID_SIZE[1], TILE_SIZE):
        for col in range(0, GRID_SIZE[0], TILE_SIZE):
            rect = pygame.Rect(col, row, TILE_SIZE, TILE_SIZE)
            if (pos := (col + move_offset[0], row + move_offset[1])) in clicked_tiles:
                pygame.draw.rect(
                    window,
                    (
                        clicked_tiles[pos][0],
                        clicked_tiles[pos][1],
                        clicked_tiles[pos][2],
                    ),
                    rect,
                    0,
                )
            else:
                pygame.draw.rect(window, DARK_GREENISH, rect, 1)


def GetMouseGridPos(mouse_pos: tuple[int, int]) -> tuple[int, int]:
    return (
        (mouse_pos[0] // TILE_SIZE) * TILE_SIZE,
        (mouse_pos[1] // TILE_SIZE) * TILE_SIZE,
    )


def RenderText(
    text: str,
    window: pygame.Surface,
    font: pygame.font.Font,
    color: list[int],
    text_pos: tuple[int, int],
) -> None:
    surface = font.render(text, True, color)
    rect = surface.get_rect()
    rect.topleft = text_pos
    window.blit(surface, rect)


def RenderColorPicker(
    window: pygame.Surface, color_pick: list[int], font: pygame.font.Font
) -> None:
    rect = pygame.rect.Rect(COLOR_PICK_RECT)
    pygame.draw.rect(window, color_pick, rect, 0)

    RenderText(f"R: {color_pick[0]}", window, font, DARK_REDDISH, R_RECT_TOPLEFT)
    RenderText(f"G: {color_pick[1]}", window, font, DARK_GREENISH, G_RECT_TOPLEFT)
    RenderText(f"B: {color_pick[2]}", window, font, DARK_BLUEISH, B_RECT_TOPLEFT)


def ChangeColorPick(
    window: pygame.Surface,
    font: pygame.font.Font,
    color_pick: list[int],
    color_change: Literal["R", "G", "B"],
) -> None:
    index = 0
    pos = tuple()
    color = list()

    if color_change == "R":
        index = 0
        pos = R_RECT_TOPLEFT
        color = DARK_REDDISH
    elif color_change == "G":
        index = 1
        pos = G_RECT_TOPLEFT
        color = DARK_GREENISH
    elif color_change == "B":
        index = 2
        pos = B_RECT_TOPLEFT
        color = DARK_BLUEISH

    input_str = f"{color_change}: {color_pick[index]}"

    while True:
        for event in pygame.event.get():
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    color_pick[index] = int(input_str[3:]) % 256
                    return
                elif event.key == pygame.K_BACKSPACE and len(input_str) > 3:
                    input_str = input_str[:-1]
                    break
                elif event.unicode.isdigit():
                    input_str += event.unicode
                    break
        else:
            continue
        pygame.draw.rect(window, WHITISH, (*pos, 200, 50))
        RenderText(input_str, window, font, color, pos)
        pygame.display.update()


def ManageClickedTiles(
    window: pygame.Surface,
    clicked_tiles: dict[tuple[int, int], list[int]],
    tile_pos: tuple[int, int],
    move_offset: list[int],
    color_pick: list[int],
    font: pygame.font.Font,
) -> None:
    if tile_pos == R_RECT_TOPLEFT:
        ChangeColorPick(window, font, color_pick, "R")
    elif tile_pos == G_RECT_TOPLEFT:
        ChangeColorPick(window, font, color_pick, "G")
    elif tile_pos == B_RECT_TOPLEFT:
        ChangeColorPick(window, font, color_pick, "B")
    if (
        tile_pos[0] > GRID_SIZE[0]
        or tile_pos[0] < 0
        or tile_pos[1] > GRID_SIZE[1]
        or tile_pos[0] < 0
    ):
        return
    abs_tile_pos = (tile_pos[0] + move_offset[0], tile_pos[1] + move_offset[1])
    if abs_tile_pos in clicked_tiles:
        clicked_tiles.pop(abs_tile_pos)
    else:
        clicked_tiles.update({abs_tile_pos: color_pick.copy()})


def GetContinuousMoveOffset(move_offset: list[int]) -> None:
    keys = pygame.key.get_pressed()

    if keys[pygame.K_w] or keys[pygame.K_UP]:
        move_offset[1] -= TILE_SIZE
    elif keys[pygame.K_s] or keys[pygame.K_DOWN]:
        move_offset[1] += TILE_SIZE
    if keys[pygame.K_a] or keys[pygame.K_LEFT]:
        move_offset[0] -= TILE_SIZE
    elif keys[pygame.K_d] or keys[pygame.K_RIGHT]:
        move_offset[0] += TILE_SIZE


# -----------------------#
#   Main app structure   #
# -----------------------#


def InitProgram() -> tuple[
    TextIO,
    pygame.Surface,
    pygame.time.Clock,
    pygame.font.Font,
    dict[tuple[int, int], list[int]],
]:
    file_path = input("Enter the file path to add to: ").strip()
    try:
        fh = open(file_path, "r+")
    except FileNotFoundError:
        raise FileNotFoundError(f"{file_path} is not a valid path.")

    pygame.init()
    window = pygame.display.set_mode(SCREEN_SIZE)
    pygame.display.set_caption("Tile Editor")

    clock = pygame.time.Clock()

    font = pygame.font.Font(abspath("assets/JetBrainMono.ttf"), 20)

    clicked_tiles = ParseWorkFile(fh)

    return fh, window, clock, font, clicked_tiles  # type: ignore


def Loop(
    window: pygame.Surface,
    clock: pygame.time.Clock,
    clicked_tiles: dict[tuple[int, int], list[int]],
    font: pygame.font.Font,
) -> None:
    move_offset = [0, 0]  # This will allow us to move across a infinite grid of tiles
    color_pick = BLACKISH

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return
            if (
                event.type == pygame.MOUSEBUTTONDOWN
                and event.button == pygame.BUTTON_LEFT
            ):
                ManageClickedTiles(
                    window,
                    clicked_tiles,
                    GetMouseGridPos(pygame.mouse.get_pos()),
                    move_offset,
                    color_pick,
                    font,
                )
        GetContinuousMoveOffset(move_offset)
        window.fill(WHITISH)
        DrawGrid(window, clicked_tiles, move_offset)
        RenderColorPicker(window, color_pick, font)
        pygame.display.flip()
        clock.tick(FPS)


def ExitProgram(fh: TextIO, clicked_tiles: dict[tuple[int, int], list[int]]) -> None:
    pygame.quit()
    SaveWorkToFile(fh, clicked_tiles)
    fh.close()
    exit()


# --------------------#
#   App entry point   #
# --------------------#


def main() -> None:
    fh, window, clock, font, clicked_tiles = InitProgram()
    Loop(window, clock, clicked_tiles, font)
    ExitProgram(fh, clicked_tiles)


if __name__ == "__main__":
    main()
