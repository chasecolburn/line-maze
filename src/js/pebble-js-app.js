var initialized = false;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  var shakeToCheat = 1; // get current setting from pebble
  Pebble.openURL('http://rawgithub.com/kesselborn/line-maze/SDK-2.0-with-shake-to-cheat-config/configuration.html?stc=' + shakeToCheat);
});

Pebble.addEventListener("appmessage", function(e) {
  console.log("Received message: " + e.payload);
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));

  console.log("sending config to pebble");
  Pebble.sendAppMessage( { "shake-to-cheat": options["shake-to-cheat"] },
                        function(e) {
                          console.log("Successfully delivered message with transactionId=" +
                                      e.data.transactionId + " ... shake-to-cheat=" + options["shake-to-cheat"]);
                        },
                        function(e) {
                          console.log("Unable to deliver message with transactionId=" +
                                      e.data.transactionId +
                                      " Error is: " + e.error.message);
                        }
                       );

  console.log("Options = " + JSON.stringify(options));
});
