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

    public static void main(String[] args) throws Exception
    {
        if (args.length < 2)
        {
            System.out.println("Usage: java SSTableGenerator <csv_dir> <output_dir>");
            return;
        }

        String csvDir = args[0];
        String outputDir = args[1];
        String outDir = outputDir + "/wow_packets/packets";
        new File(outDir).mkdirs();

        CQLSSTableWriter writer = CQLSSTableWriter.builder()
            .inDirectory(outDir)
            .forTable(SCHEMA)
            .using(INSERT)
            .withMaxSSTableSizeInMiB(256)
            .build();

        for (File csvFile : new File(csvDir).listFiles((d, name) -> name.endsWith(".csv")))
        {
            System.out.println("Processing: " + csvFile.getName());
            
            try (BufferedReader br = new BufferedReader(new FileReader(csvFile)))
            {
                String line;
                while ((line = br.readLine()) != null)
                {
                    String[] cols = line.split(",", 9);
                    writer.addRow(
                        Integer.parseInt(cols[0]), // build
                        UUID.fromString(cols[1]), // file_id
                        Integer.parseInt(cols[2]), // bucket
                        Integer.parseInt(cols[3]), // packet_number
                        Byte.parseByte(cols[4]), // direction
                        Integer.parseInt(cols[5]), // packet_len
                        Integer.parseInt(cols[6]), // opcode
                        Long.parseLong(cols[7]), // timestamp
                        ByteBuffer.wrap(Base64.getDecoder().decode(cols[8])) // pkt_json blob
                    );
                }
            }
        }

        writer.close();
        System.out.println("SSTables written to: " + outDir);
    }
}