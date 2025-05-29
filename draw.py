#!/usr/bin/python3
import pygame
import threading
import subprocess
import os

def compile_source():
    src = "main.c"
    output = "a.out"
    if os.path.exists(src):
        print(f"Compiling {src} -> {output}")
        result = subprocess.run(["gcc", src, "-o", output])
        if result.returncode != 0:
            print("Compilation failed.")
            exit(1)
    else:
        print("main.c not found.")
        exit(1)

latest_segments = [[0] * 8 for _ in range(8)]

def draw_one_digit(screen, x, y, size, segments):
    color_on = (255, 0, 0)
    color_off = (30, 0, 0)
    w = size
    h = size // 5
    offset = size // 6

    rects = [
        pygame.Rect(x + offset, y, w, h),                     # a
        pygame.Rect(x + w + offset, y + offset, h, w),        # b
        pygame.Rect(x + w + offset, y + w + offset*2, h, w),  # c
        pygame.Rect(x + offset, y + w*2 + offset*2, w, h),    # d
        pygame.Rect(x, y + w + offset*2, h, w),               # e
        pygame.Rect(x, y + offset, h, w),                     # f
        pygame.Rect(x + offset, y + w + offset, w, h),        # g
    ]

    for i in range(7):
        pygame.draw.rect(screen, color_on if segments[i] else color_off, rects[i])

    if segments[7]:
        pygame.draw.circle(screen, color_on, (x + w + 2 * offset, y + w * 2 + offset * 2 + 5), 5)

def draw_digits(screen, segment_list):
    screen.fill((0, 0, 0))
    for i, seg in enumerate(segment_list):
        draw_one_digit(screen, x=50 + i * 150, y=50, size=60, segments=seg)
    pygame.display.flip()

def read_subprocess():
    global latest_segments
    proc = subprocess.Popen(["./a.out"], stdout=subprocess.PIPE, text=True)
    for line in proc.stdout:
        if line.startswith("DIGITAL:"):
            part = line.strip().split("{")[-1].replace("}", "").strip()
            seg_list = []
            for hex_str in part.split():
                value = int(hex_str, 16)
                bits = [(value >> i) & 1 for i in reversed(range(8))]
                seg_list.append(bits)
            latest_segments = seg_list

def main():
    pygame.init()
    screen = pygame.display.set_mode((1600, 250))
    pygame.display.set_caption("Seven Segment Realtime Display")

    thread = threading.Thread(target=read_subprocess, daemon=True)
    thread.start()

    clock = pygame.time.Clock()

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return

        draw_digits(screen, latest_segments)
        clock.tick(10)

if __name__ == "__main__":
    compile_source()
    main()
