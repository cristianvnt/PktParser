import org.apache.cassandra.io.sstable.CQLSSTableWriter;
import java.io.*;
import java.util.Base64;
import java.nio.ByteBuffer;
import java.util.UUID;

public class SSTableGenerator
{
    static final String SCHEMA = "CREATE TABLE wow_packets.packets ("
        + "build int, "
        + "file_id uuid, "
        + "bucket int, "
        + "packet_number int, "
        + "direction tinyint, "
        + "packet_len int, "
        + "opcode int, "
        + "timestamp bigint, "
        + "pkt_json blob, "
        + "PRIMARY KEY ((build, file_id, bucket), packet_number)"
        + ") WITH CLUSTERING ORDER BY (packet_number ASC)";

    static final String INSERT = "INSERT INTO wow_packets.packets "
        + "(build, file_id, bucket, packet_number, direction, packet_len, opcode, timestamp, pkt_json) "
        + "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    static void ProcessCSVs(String csvDir, String outputDir) throws Exception
    {
        String outDir = outputDir + "/wow_packets/packets";
        new File(outDir).mkdirs();

        CQLSSTableWriter writer = CQLSSTableWriter.builder()
            .inDirectory(outDir)
            .forTable(SCHEMA)
            .using(INSERT)
            .withMaxSSTableSizeInMiB(256)
            .build();

        int totalRows = 0;
        int skippedRows = 0;

        File[] csvFiles = new File(csvDir).listFiles((d, name) -> name.endsWith(".csv"));
        if (csvFiles != null)
        {
            for (File csvFile : csvFiles)
            {
                System.err.println("Processing: " + csvFile.getName());
                int lineNum = 0;

                try (BufferedReader br = new BufferedReader(new FileReader(csvFile)))
                {
                    String line;
                    while ((line = br.readLine()) != null)
                    {
                        lineNum++;
                        try
                        {
                            String[] cols = line.split(",", 9);
                            if (cols.length < 9 || cols[8].isEmpty())
                            {
                                System.err.println("WARN: Skipping malformed line " + lineNum + " in " + csvFile.getName());
                                skippedRows++;
                                continue;
                            }

                            writer.addRow(
                                Integer.parseInt(cols[0]),
                                UUID.fromString(cols[1]),
                                Integer.parseInt(cols[2]),
                                Integer.parseInt(cols[3]),
                                Byte.parseByte(cols[4]),
                                Integer.parseInt(cols[5]),
                                Integer.parseInt(cols[6]),
                                Long.parseLong(cols[7]),
                                ByteBuffer.wrap(Base64.getDecoder().decode(cols[8]))
                            );
                            totalRows++;
                        }
                        catch (Exception e)
                        {
                            System.err.println("WARN: Skipping bad line " + lineNum + " in " + csvFile.getName() + ": " + e.getMessage());
                            skippedRows++;
                        }
                    }
                }
            }
        }

        writer.close();
        System.err.println("SSTables written: " + totalRows + " rows, " + skippedRows + " skipped");
    }

    public static void main(String[] args) throws Exception
    {
        if (args.length >= 2 && !args[0].equals("--daemon"))
        {
            ProcessCSVs(args[0], args[1]);
            return;
        }

        if (args.length == 1 && args[0].equals("--daemon"))
        {
            // stdout is ONLY for signals - everything else goes to stderr
            System.out.println(">>READY<<");
            System.out.flush();

            BufferedReader stdin = new BufferedReader(new InputStreamReader(System.in));
            String line;
            while ((line = stdin.readLine()) != null)
            {
                line = line.trim();
                if (line.equals("QUIT"))
                    break;

                String[] parts = line.split(" ", 2);
                if (parts.length != 2)
                {
                    System.err.println("Bad input: " + line);
                    System.out.println(">>ERROR: expected <csv_dir> <output_dir><<");
                    System.out.flush();
                    continue;
                }

                try
                {
                    ProcessCSVs(parts[0], parts[1]);
                    System.out.println(">>DONE<<");
                }
                catch (Exception e)
                {
                    System.err.println("SSTable generation failed: " + e.getMessage());
                    e.printStackTrace(System.err);
                    System.out.println(">>ERROR: " + e.getMessage() + "<<");
                }
                System.out.flush();
            }
            return;
        }

        System.err.println("Usage: java -jar sstable.jar <csv_dir> <output_dir> | --daemon");
    }
}