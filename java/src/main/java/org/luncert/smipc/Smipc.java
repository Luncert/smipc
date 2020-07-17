package org.luncert.smipc;

import org.luncert.smipc.constants.ChannelMode;
import org.luncert.smipc.constants.LogMode;
import org.luncert.smipc.exception.SmipcException;

import java.io.Closeable;
import java.io.IOException;

import static org.luncert.smipc.SmipcLib.OP_SUCCEED;

/**
 * TODO: add onChannelData, removeListener
 */
public final class Smipc {
  
  private static volatile boolean initialized = false;
  
  private Smipc() {}
  
  /**
   * Initialize library data, set log level and register Runtime hook to clean library.
   * @param logMode {@link org.luncert.smipc.constants.LogMode}
   * @throws SmipcException this method could be invoked for only one time, otherwise it will throw exception.
   */
  public static void init(LogMode logMode) throws SmipcException {
    if (initialized) {
      throw new SmipcException("library has been initialized");
    }
    
    SmipcLib.initLibrary(logMode.getLogMode());
    Runtime.getRuntime().addShutdownHook(new Thread(SmipcLib::cleanLibrary));
    initialized = true;
  }
  
  /**
   * Open new channel with provided configurations.
   * @param cid channel id, all opened channel will be saved to a internal map,
   *                  thus provided id must be different from all the existing's
   * @param mode channel mode, READ or WRITE, see {@link org.luncert.smipc.constants.ChannelMode}
   * @param channelSize channel size, in other words, the shared memory's capacity
   * @return {@link org.luncert.smipc.Smipc.Channel}
   * @throws IOException if operation failed, throw {@code IOException}
   */
  public static Channel open(String cid, ChannelMode mode, int channelSize) throws IOException {
    int ret = SmipcLib.openChannel(cid, mode.getMode(), channelSize);
    checkOperationRetVal(ret, "failed to open channel");
    
    return new Channel(cid, mode);
  }
  
  public static class Channel implements Closeable {
    
    private final String cid;
    private final ChannelMode mode;
    
    Channel(String cid, ChannelMode mode) {
      this.cid = cid;
      this.mode = mode;
    }
    
    synchronized public void printChannelStatus() throws IOException {
      int ret = SmipcLib.printChannelStatus(cid);
      checkOperationRetVal(ret, "unable to print channel status");
    }
  
    /**
     * Write data to channel.
     * @param data data
     * @param start inclusive start position
     * @param end exclusive end position
     */
    synchronized public void write(byte[] data, int start, int end) throws IOException {
      if (!ChannelMode.WRITE.equals(mode)) {
        throw new SmipcException("unsupported write operation on read end channel");
      }
      
      int ret = SmipcLib.writeChannel(cid, data, start, end);
      checkOperationRetVal(ret, "failed to write channel");
    }
  
    /**
     * Read data from channel.
     * @param dest destination buffer
     * @param start inclusive start position
     * @param end exclusive end position
     * @param blocking if {@code True}, channel will try to read enough bytes indicating by {@code start}
     *                 and {@code end}, if no more data available in shared memory, thread will be blocked
     *                 until new data coming
     */
    synchronized public void read(byte[] dest, int start, int end, boolean blocking) throws IOException {
      if (!ChannelMode.READ.equals(mode)) {
        throw new SmipcException("unsupported read operation on write end channel");
      }
      
      int len = end - start;
      if (len > dest.length || len < 0) {
        throw new SmipcException("required bytes to read exceeds dest buffer's size or is negative");
      }
  
      int ret = SmipcLib.readChannel(cid, dest, start, end, blocking);
      checkOperationRetVal(ret, "failed to read channel");
    }
    
    // TODO: thread-safe
    @Override
    public void close() {
      SmipcLib.closeChannel(cid);
    }
  }
  
  private static void checkOperationRetVal(int ret, String msg) throws SmipcException {
    if (ret < OP_SUCCEED) {
      throw new SmipcException(msg + ", " + SmipcLib.OP_RESULT_NAMES[Math.abs(ret)]);
    }
  }
}
