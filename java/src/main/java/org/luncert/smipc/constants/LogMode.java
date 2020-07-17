package org.luncert.smipc.constants;

public enum LogMode {
  
  DISABLE(0),
  BASIC(1),
  ALL(2);
  
  private int logMode;
  LogMode(int logMode) {
    this.logMode = logMode;
  }
  
  public int getLogMode() {
    return logMode;
  }
}
