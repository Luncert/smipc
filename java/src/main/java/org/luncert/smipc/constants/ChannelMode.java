package org.luncert.smipc.constants;

public enum ChannelMode {
  
  READ(0),
  WRITE(1);
  
  private int mode;
  ChannelMode(int mode) {
    this.mode = mode;
  }
  
  public int getMode() {
    return mode;
  }
}
