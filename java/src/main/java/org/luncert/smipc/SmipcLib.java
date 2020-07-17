package org.luncert.smipc;

import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.Paths;
import java.util.Objects;

// Ref: https://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/jniTOC.html
public final class SmipcLib {
  
  public static final int OP_SUCCEED = 0;
  public static final int OP_FAILED = -1;
  public static final int OPPOSITE_END_CLOSED = -2;
  public static final String[] OP_RESULT_NAMES = new String[]{
      "operation succeed",
      "operation failed,",
      "opposite end has been closed"
  };
  
  public static final int LOG_DISABLE = 0;
  public static final int LOG_BASIC = 1;
  public static final int LOG_ALL = 2;
  
  public static final int CHAN_READ = 0;
  public static final int CHAN_WRITE = 1;
  
  native static void initLibrary(int logMode);
  
  native static void cleanLibrary();
  
  native static int openChannel(String cid, int channelMode, int channelSize);
  
  native static int writeChannel(String cid, byte[] data, int start, int end);
  
  native static int readChannel(String cid, byte[] data, int start, int end, boolean blocking);
  
  native static int printChannelStatus(String cid);
  
  native static void closeChannel(String cid);
  
  static {
    URL url = SmipcLib.class.getResource("/lib/libsmipc.dll");
    Objects.requireNonNull(url);
    
    try {
      System.load(Paths.get(url.toURI()).toString());
    } catch (URISyntaxException e) {
      throw new RuntimeException(e);
    }
  }
}
