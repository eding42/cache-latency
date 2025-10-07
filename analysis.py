import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
rng = np.random.default_rng(12345)

files = {
    "(a) Random,  Cache ON":  ("random_on.csv", 19),
    "(a) Random,  Cache OFF": ("random_off.csv", 12),
    "(b) Locality, Cache ON": ("locality_on.csv", 19),
    "(b) Locality, Cache OFF":("locality_off.csv", 12),
}

def load(fname):
    return np.loadtxt(fname, dtype=int)

def ecdf(x):
    x = np.sort(x)
    y = np.arange(1, len(x)+1) / len(x)
    return x, y

def bootstrap_ci(stat_fn, data, B=1000, alpha=0.05):
    n = len(data)
    idx = rng.integers(0, n, size=(B, n))
    samples = data[idx]
    stats = np.apply_along_axis(stat_fn, 1, samples)
    lo = np.quantile(stats, alpha/2)
    hi = np.quantile(stats, 1 - alpha/2)
    return lo, hi

def frac_lt_threshold(data, thr):
    return np.mean(data < thr)

# ------------- Plot CDFs -------------
fig1 = plt.figure()
for label, (fname, _) in files.items():
    x = load(fname)
    xs, ys = ecdf(x)
    plt.plot(xs, ys, label=label)
plt.xlabel("Access time (cycles)")
plt.ylabel("Empirical CDF")
plt.title("CDFs of Access Time (Parts a & b)")
plt.legend()
plt.tight_layout()
plt.savefig("cdf_access_times.png", dpi=200)
plt.close()

print("Wrote plot to cdf_access_times.png")

# ------------- Confidence limits & fractions -------------
print("\n=== Confidence limits (bootstrap, 95%) & worst-case fractions ===")
for label, (fname, worst) in files.items():
    x = load(fname).astype(float)
    mean = x.mean()
    lo, hi = bootstrap_ci(np.mean, x, B=2000)
    frac_lt = frac_lt_threshold(x, worst)
    # Optional CI for the fraction itself:
    flo, fhi = bootstrap_ci(lambda arr: np.mean(arr < worst), x, B=2000)

    print(f"\n{label}")
    print(f"  Mean cycles: {mean:.6f}  (95% CI: {lo:.6f}, {hi:.6f})")
    print(f"  Fraction cycles < worst-case ({worst}): {frac_lt:.6f}  (95% CI: {flo:.6f}, {fhi:.6f})")
