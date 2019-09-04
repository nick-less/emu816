
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Map.Entry;

public class MakeSnippet {

	static void copy(InputStream in, OutputStream out) throws IOException {

		byte[] buffer = new byte[1024];
		int lengthRead;
		while ((lengthRead = in.read(buffer)) > 0) {
			out.write(buffer, 0, lengthRead);
			out.flush();
		}
	}

	
	
	
	
	public static void main(String[] args) {
		try (InputStream inputStream = new FileInputStream("loader")) {
			ByteArrayOutputStream out = new ByteArrayOutputStream();
			copy(inputStream, out);
			byte data[] = out.toByteArray();

			Map<String, Set<String>> valueMap = new HashMap<>();

			for (int i=0;i<data.length;i++) {
				if (data[i] != 0) {
					String v = String.format("%02X", data[i]);

					Set<String> addrs = valueMap.computeIfAbsent(v, k->new HashSet<String>());
					
					String a =  String.format("%7s",
							  Integer.toBinaryString(i+4)).replaceAll(" ", "0");
					addrs.add(a);
				}
			}
			for (Entry<String, Set<String>> entry : valueMap.entrySet()) {
				String a = "";
				for (String s: entry.getValue()) {
					a += "\""+s+"\"";
					a += " | ";
				}
				a = a.substring(0, a.length()-3);
			System.out.println("when "+a+" => int_dout <= x\""+entry.getKey()+"\";");

			}

			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}