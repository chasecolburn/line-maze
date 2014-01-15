var initialized = false;
var shakeToCheat = localStorage.getItem("shake-to-cheat") || 1;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
  Pebble.sendAppMessage( { "js-initialized": 1 },
                        function(e) {
                          console.log("Successfully told pebble that I am ready");
                        },
                        function(e) {
                          console.log("Failed telling pebble that I am ready: "+ e.error.message);
                        }
                       );
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('http://rawgithub.com/kesselborn/line-maze/SDK-2.0-with-shake-to-cheat-config/configuration.html?stc=' + shakeToCheat);
});

  Pebble.addEventListener("appmessage", function(e) {
    var command = e.payload["command"];

    if(command === "send-config") {
      console.log("sending config to pebble as requested");
      setShakeToCheat(shakeToCheat);
    }
  });

  function setShakeToCheat(shakeToCheat) {
    Pebble.sendAppMessage( { "shake-to-cheat": shakeToCheat},
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

  }

  Pebble.addEventListener("webviewclosed", function(e) {
    console.log("configuration closed");
    // webview closed
    var options = JSON.parse(decodeURIComponent(e.response));

    console.log("sending config to pebble");
    localStorage.setItem("shake-to-cheat", options["shake-to-cheat"]);
    setShakeToCheat(options["shake-to-cheat"]);

    console.log("Options = " + JSON.stringify(options));
  });
