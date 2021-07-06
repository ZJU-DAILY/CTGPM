package com.xwcai;
import java.io.*;
import java.util.*;

import javafx.util.Pair;
import org.jgrapht.*;
import org.jgrapht.graph.*;
import org.jgrapht.generate.*;

public class DataGenetator {
    public static void main(String[] args) throws Exception {
        Scanner scan = new Scanner(System.in);
        System.out.print("Label set: ");
        String labelFile = scan.nextLine();
        System.out.print("|V| = ");
        int vertices = scan.nextInt();
        System.out.print("α = ");
        double alpha = scan.nextDouble();

        Generate(labelFile, vertices, alpha);
    }

    public static void Generate(String labelFile, int vertices, double alpha) throws IOException{
        int edges = (int)Math.floor(Math.pow(vertices, alpha));

        BufferedReader bf = new BufferedReader(new FileReader("labels/" + labelFile + ".label"));
        ArrayList<Pair<Integer, String>> label_set = new ArrayList<>();
        int sum = 0;
        for (String line; (line = bf.readLine()) != null;) {
            String label = line.substring(0, line.indexOf(" "));
            Integer num = new Integer(line.substring(line.indexOf(" ") + 1));
            sum += num;
            label_set.add(new Pair<>(num, label));
        }
        bf.close();

        String[] label_map = new String[vertices];
        for (int i = 0; i < vertices; ++i) {
            int rand = (int) (Math.random() * sum);
            for (Pair<Integer, String> integerStringPair : label_set) {
                rand -= integerStringPair.getKey();
                if (rand < 0) {
                    label_map[i] = integerStringPair.getValue();
                    break;
                }
            }
        }

        GnmRandomGraphGenerator<Integer, DefaultEdge> gen = new GnmRandomGraphGenerator<>(vertices, edges);
        Graph<Integer, DefaultEdge> RandomDataGraph = new DefaultDirectedGraph<>(DefaultEdge.class);
        VertexFactory vertexFactory = new VertexFactory() {
            Integer seq = -1;

            @Override
            public Object createVertex() {
                return (++seq);
            }
        };

        gen.generateGraph(RandomDataGraph, vertexFactory, null);

        Set<DefaultEdge> all_edges = RandomDataGraph.edgeSet();
        FileWriter fw = new FileWriter("datas/" + vertices + "-" + alpha + ".data");

        for (DefaultEdge edge : all_edges) {
            String edge_str = edge.toString();
            Integer src = new Integer(edge_str.substring(edge_str.indexOf("(") + 1, edge_str.indexOf(" ")));
            Integer dst = new Integer(edge_str.substring(edge_str.indexOf(":") + 2, edge_str.indexOf(")")));

            int start = (int)(Math.random() * 87600);
            int end = start + 1;

            fw.write(src + " " + dst + "\n");
        }

        fw.flush();
        fw.close();
    }
}
