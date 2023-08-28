int main() {
    int pt1, pt2, t, N;

    for (int i = 0; i < N; i++)
    {
        pt2 = F(pt1, pt2);

        t = pt1;
        pt1 = pt2;
        pt2 = t;
    }

    for (int i = 0; i < N; i += 2)
    {
        pt2 = F(pt1, pt2);
        pt1 = F(pt2, pt1);
    }
}
