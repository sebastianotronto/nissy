import os
import subprocess
import sys
from collections import defaultdict
from dataclasses import dataclass

UD = {"U","D","U'","D'"}
#NISSY_EXEC = "nissy"
NISSY_EXEC = "/Users/rodneykinney/workspace/nissy/nissy"

def nissy(*args):
    cmd = [NISSY_EXEC] + list(args)
    output = subprocess.check_output(cmd, encoding="UTF8")
    return output


def parse_nissy_output(s):
    parts = s.strip().split(' ')
    moves = parts[:-1]
    move_count = parts[-1].replace('(','').replace(')','')
    return moves, int(move_count)
    

def analyze(sol):
    moves, move_count = parse_nissy_output(sol)
    ud_move_count = sum(1 for i in range(0, len(moves)) if moves[i] in UD and (i == 0 or moves[i-1] not in UD) and (i == move_count-1 or moves[i+1] not in UD))
    solution = " ".join(moves)
    scramble = nissy("invert",solution)
    corner_moves, corner_move_count = parse_nissy_output(nissy("solve","corners-drud",scramble))
    corner_solution = " ".join(corner_moves)
    corner_ud_move_count = sum(1 for m in corner_moves if m in UD)
    return "{}\t{}\t{}\t{}\t{}\t{}".format(solution, move_count, ud_move_count, corner_solution, corner_move_count, corner_ud_move_count)

def random_dr():
    scramble = nissy("scramble").strip()
    mv,mc = parse_nissy_output(nissy("solve", "drud", scramble))
    dr_scramble = "{} {}".format(scramble, " ".join(mv))
    solution = nissy("solve","drudfin", dr_scramble)
    return solution

def histograms(lines):
    overall_counts = defaultdict(lambda : 0)
    for l in lines:
        overall_counts[l[1]] += 1
    ud_mc = set(l[2] for l in lines)
    counts_by_udmc = defaultdict(lambda : defaultdict(lambda : 0))
    for mc in ud_mc:
        for l in lines:
            if l[2] == mc:
                counts_by_udmc[mc][l[1]] += 1
    return overall_counts, counts_by_udmc

def generate(rows):
    print("solution\tmove_count\tud_move_count\tcorner_solution\tcorner_move_count\tcorner_ud_move_count")
    for i in range(0,rows):
        print(analyze(random_dr()))
        sys.stdout.flush()

@dataclass
class Row:
    solution: str
    move_count: int
    ud_move_count: int
    corner_solution: str
    corner_move_count: int
    corner_ud_move_count: int

    @classmethod
    def parse(cls, s):
        fields = s.strip().split('\t')
        corner_ud_mc = int(fields[5])
        corner_ud_mc = 4 if corner_ud_mc == 6 else corner_ud_mc
        corner_ud_mc = 5 if corner_ud_mc == 7 else corner_ud_mc
        return Row(fields[0],
                   int(fields[1]),
                   int(fields[2]),
                   fields[3],
                   int(fields[4]),
                   corner_ud_mc
                   )
    

def histogram(rows):
    rows = sorted(rows, key = lambda r: r.move_count)
    mc_hist = {}
    for r in rows:
        mc_hist[r.move_count] = mc_hist.get(r.move_count, 0) + 1
    last_row = rows[0]
    for r in rows[1:]:
        if r.move_count == int(last_row.move_count):
            r.move_count = last_row.move_count + 1.0 / mc_hist[r.move_count]
        last_row = r
        
    return rows, mc_hist

def range(rows):
    p50 = rows[int(len(rows)/2)].move_count
    p20 = rows[int(len(rows)/5)].move_count
    p80 = rows[int(4 * len(rows)/5.0)].move_count
    return p20, p50, p80
    

def read(file):
    return [Row.parse(l) for l in open(file).readlines()[1:]]

def statistics(file):
    rows, mc_hist = histogram(read(file))
    print("Corner QTs\tp20\tp50\tp80")
    ud_mc = sorted(list({r.corner_ud_move_count for r in rows}))
    for mc in ud_mc:
        p20, p50, p80 = range([r for r in rows if int(r.corner_ud_move_count) == mc])
        print(f"{mc}\t{p20:.1f}\t{p50:.1f}\t{p80:.1f}")
    print("")
    p20, p50, p80 = range(rows)
    print(f"Overall\t{p20:.1f}\t{p50:.1f}\t{p80:.1f}")

    print("")
    print("")
    print("Corner QTs\tExtra U turns")
    for mc in ud_mc:
        rows_with_ud_mc = [r for r in rows if r.corner_ud_move_count == mc]
        print(f"{mc}\t{100 * len([r for r in rows_with_ud_mc if r.ud_move_count != r.corner_ud_move_count])/len(rows_with_ud_mc):.0f}%")

    print("")
    print("")
    print("Corner QTs\tFirst/List is U/D")
    for mc in ud_mc:
        first_moves = [r.solution.split(" ")[0] for r in rows if r.corner_ud_move_count == mc]
        last_moves = [r.solution.split(" ")[-1] for r in rows if r.corner_ud_move_count == mc]
        first_is_ud = sum(1 for m in first_moves if m in UD)
        last_is_ud = sum(1 for m in last_moves if m in UD)
        print(f"{mc}\t{int((first_is_ud + last_is_ud) / (len(first_moves) + len(last_moves)) * 100)}%")


if __name__ == "__main__":
    rows = int(sys.argv[1]) if len(sys.argv) > 1 else 10
    #generate(rows)
    #statistics(sys.argv)

