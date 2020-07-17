package org.luncert.smipc;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.luncert.smipc.constants.ChannelMode;
import org.luncert.smipc.constants.LogMode;

import java.io.IOException;

@RunWith(JUnit4.class)
public class SmipcTest {
  
  private String channelId = "test-chan";
  private int chanSize = 11;
  private String testData = "Hello, world!";
  
  @Test
  public void testWrite() throws IOException {
    byte[] buffer = testData.getBytes();
  
    Smipc.init(LogMode.ALL);
    try (Smipc.Channel channel = Smipc.open(channelId, ChannelMode.WRITE, chanSize)) {
      channel.write(buffer, 0, buffer.length);
    }
  }
  
  @Test
  public void testRead() throws IOException {
    byte[] buffer = new byte[20];
    Smipc.init(LogMode.ALL);
    try (Smipc.Channel channel = Smipc.open(channelId, ChannelMode.READ, chanSize)) {
      channel.read(buffer, 0, testData.length(), true);
    }
  
    Assert.assertEquals(testData, new String(buffer));
  }
}
