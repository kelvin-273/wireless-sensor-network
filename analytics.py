import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("output.csv")
fisrt = lambda x: x.head(1)
last = lambda x: x.tail(1)
time_per_cycle = lambda df: last(df["elapsed_time"]) / last(df["cycle_no"])
count_events = lambda df: [len(df.loc[df["node_rank"] == i]) for i in range(60)]

def quicksort(A):
    if A == []:
        return []
    else:
        return quicksort([i for i in A[1:] if i < A[0]]) + \
        [A[0]] + quicksort([i for i in A[1:] if i >= A[0]])

def normalise(A):
    factor = sum(A)
    return [i/factor for i in   A]

def split(A, n):
    if len(A) == 0:
        return []
    if len(A) < n:
        return [A]
    else:
        return [A[:n]] + split(A[n:], n)

def is_edge(x, dimx):
    return (x == dimx - 1) or (x == 0)

def where(coords, dims):
    assert len(coords) == len(dims)
    bools = [is_edge(coords[i], dims[i]) for i in range(len(coords))]
    if all(bools):
        return "corner"
    elif any(bools):
        return "edge"
    else:
        return "centre"

def convert(string):
    return [int(i) for i in string.replace("{","").replace("}","").split(",")][1:]

get_category = lambda x: where(convert(x), (4, 15))
df["category"] = pd.Series(map(get_category, df["node_key"]))

if __name__ == '__main__':
    plt.clf()
    plt.xkcd()
    plt.set_cmap("inferno")
    plt.figure(1)
    plt.title("Inverse CMD of sorted number of events")
    plt.plot(normalise(quicksort(count_events(df))))
    plt.figure(2)
    plt.hist(normalise(count_events(df)))
    plt.matshow(split(count_events(df), 15))
    plt.title("Event detection heatmap")
    plt.show()
