package com.xwcai;

import javafx.util.Pair;

import java.io.*;
import java.util.*;

public class PatternGenerator {
    public static void main(String[] args) throws Exception {
        Scanner scan = new Scanner(System.in);
        String labelFile = "ML3000";
        for (int vertices = 3; vertices <= 8; ++vertices) {
            float alpha = (float) 1.15;
            int pnum = 25;

            System.out.println("Pattern set " + labelFile + "-" + vertices + "-" + alpha + "-" + "X" + ".p" + " is generating...");
            Generate(labelFile, vertices, alpha, pnum);
        }

        for (int rat = 1; rat <= 5; ++rat) {
            float alpha = (float) (1.0 + rat * 0.05);
            int vertices = 6;
            int pnum = 25;

            System.out.println("Pattern set " + labelFile + "-" + vertices + "-" + alpha + "-" + "X" + ".p" + " is generating...");
            Generate(labelFile, vertices, alpha, pnum);
        }
    }

    public static void Generate(String labelFile, int vertices, float alpha, int pnum) throws IOException {
//        BufferedReader bf = new BufferedReader(new FileReader("labels/" + labelFile + ".label"));
//        ArrayList<Pair<Integer, String>> label_set = new ArrayList<>();
//        int sum = 0;
//        for (String line; (line = bf.readLine()) != null;) {
//            String label = line.substring(0, line.indexOf(" "));
//            Integer num = new Integer(line.substring(line.indexOf(" ") + 1));
//            sum += num;
//            label_set.add(new Pair<>(num, label));
//        }
//        bf.close();

        int left_pat = pnum;
        while (left_pat > 0) {
            String[] label_map = new String[vertices];
            for (int i = 0; i < vertices; ++i) {
//                int rand = (int) (Math.random() * sum);
//                for (Pair<Integer, String> integerStringPair : label_set) {
//                    rand -= integerStringPair.getKey();
//                    if (rand <= 0) {
//                        label_map[i] = integerStringPair.getValue();
//                        break;
//                    }
//                }
                 int rand = (int) (Math.random() * 3000);
                 label_map[i] = "L" + String.valueOf(rand);
            }
            if (vertices == 2) {
                left_pat--;
                FileWriter fw = new FileWriter("patterns/" + labelFile + "/" + vertices + "-" + alpha
                        + "/" + left_pat + ".p");
                for (int i = 0; i < vertices; ++i) {
                    fw.write("node " + i + " " + label_map[i] + "\n");
                }
                fw.write("edge 0 1\n");
                fw.close();
                continue;
            }
            int[] f = new int[vertices];
            for (int i = 0; i < vertices; ++i) {
                f[i] = i;
            }
            boolean[][] used = new boolean[vertices][vertices];
            int left_edge = (int)Math.floor(Math.pow(vertices, alpha));
            ArrayList<Pair<Integer, Integer>> edges = new ArrayList<>();
            while (left_edge > 0) {
                int from = (int) (Math.random() * vertices);
                int to = (int) (Math.random() * vertices);
                if (from == to) continue;
                if (from > to) {
                    int tmp = from;
                    from = to;
                    to = tmp;
                }
                if (used[from][to]) continue;
                edges.add(new Pair<>(from, to));
                used[from][to] = true;
                un(f, from, to);
                --left_edge;
            }
            int cnt_group = 0;
            for (int i = 0; i < vertices; ++i) {
                if (sf(f, i) == i) cnt_group++;
            }
            if (cnt_group != 1) continue;
            int[] in = new int[vertices];
            for (Pair<Integer, Integer> edge : edges) {
                in[edge.getValue()]++;
            }
            boolean flag = true;
            for (int i = 1; i < vertices; ++i) {
                if (in[i] == 0) flag = false;
            }
            if (!flag) continue;
            --left_pat;
            FileWriter fw = new FileWriter("patterns/" + labelFile + "/" + vertices + "-" + alpha
                    + "/" + left_pat + ".p");
            for (int i = 0; i < vertices; ++i) {
                fw.write("node " + i + " " + label_map[i] + "\n");
            }
            for (Pair<Integer, Integer> edge : edges) {
                fw.write("edge " + edge.getKey() + " " + edge.getValue() + "\n");
            }
            fw.close();
        }

    }

    public static int sf(int[] f, int x) {
        if (x == f[x]) return x;
        return f[x] = sf(f, f[x]);
    }

    public static void un(int[] f, int a, int b) {
        int aa = sf(f, a), bb = sf(f, b);
        if (aa == bb) return;
        f[aa] = bb;
    }
}
