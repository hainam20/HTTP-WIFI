$(document).ready(function () {
  //   getConnectInfo();
  $("#connect_wifi").on("click", function () {
    checkCredentials();
    // alert("Button clicked!");
  });
  $("#disconnect_wifi").on("click", function () {
    disconnectWifi();
  });
  $("#connect_mqtt").on("click", function () {
    checkMQTTCredentials();
    alert("Button clicked!");
  });
  $("#NumberNodes").submit(function () {
    numberNodes();
    // alert("Button clicked!");
  });
});

function numberNodes() {
  // Get the SSID and password
  quantity = $("#quantity").val();
  console.log(quantity);
  $.ajax({
    url: "/quantity.json",
    dataType: "json",
    method: "POST",
    cache: false,
    headers: { "/quantity/": quantity },
    data: { timestamp: Date.now() },
  });
}
/**
 * Gets the WiFi connection status.
 */
function getWifiConnectStatus() {
  var xhr = new XMLHttpRequest();
  var requestURL = "/wifiConnectStatus";
  xhr.open("POST", requestURL, false);
  xhr.send("wifi_connect_status");

  if (xhr.readyState == 4 && xhr.status == 200) {
    var response = JSON.parse(xhr.responseText);

    document.getElementById("wifi_connect_status").innerHTML = "Connecting...";

    if (response.wifi_connect_status == 2) {
      document.getElementById("wifi_connect_status").innerHTML =
        "<h4 class='rd'>Failed to Connect. Please check your AP credentials and compatibility</h4>";
      stopWifiConnectStatusInterval();
    } else if (response.wifi_connect_status == 3) {
      document.getElementById("wifi_connect_status").innerHTML =
        "<h4 class='gr'>Connection Success!</h4>";
      stopWifiConnectStatusInterval();
    }
  }
}
function stopWifiConnectStatusInterval() {
  if (wifiConnectInterval != null) {
    clearInterval(wifiConnectInterval);
    wifiConnectInterval = null;
  }
}
function startWifiConnectStatusInterval() {
  wifiConnectInterval = setInterval(getWifiConnectStatus, 2800);
}
function connectWifi() {
  // Get the SSID and password
  selectedSSID = $("#connect_ssid").val();
  pwd = $("#connect_pass").val();
  $.ajax({
    url: "/wifiConnect.json",
    dataType: "json",
    method: "POST",
    cache: false,
    headers: { "my-connect-ssid": selectedSSID, "my-connect-pwd": pwd },
    data: { timestamp: Date.now() },
  });

  startWifiConnectStatusInterval();
}

function checkCredentials() {
  errorList = "";
  credsOk = true;

  selectedSSID = $("#connect_ssid").val();
  pwd = $("#connect_pass").val();
  if (selectedSSID == "") {
    errorList += "<h4 class='rd'>SSID cannot be empty!</h4>";
    credsOk = false;
  }
  if (pwd == "") {
    errorList += "<h4 class='rd'>Password cannot be empty!</h4>";
    credsOk = false;
  }

  if (credsOk == false) {
    $("#wifi_connect_credentials_errors").html(errorList);
  } else {
    $("#wifi_connect_credentials_errors").html("");
    connectWifi();
  }
}
/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword() {
  var x = document.getElementById("connect_pass");
  if (x.type === "password") {
    x.type = "text";
  } else {
    x.type = "password";
  }
}

function disconnectWifi() {
  $.ajax({
    url: "/wifiDisconnect.json",
    dataType: "json",
    method: "DELETE",
    cache: false,
    data: { timestamp: Date.now() },
  });
  // Update the web page
  setTimeout("location.reload(true);", 2000);
}

function checkMQTTCredentials() {
  errorList = "";
  credsOk = true;

  selectedhost = $("#host").val();
  username = $("#username").val();
  pwd = $("#password").val();
  if (username == "") {
    errorList += "<h4 class='rd'>Username cannot be empty!</h4>";
    credsOk = false;
  }
  if (selectedhost == "") {
    errorList += "<h4 class='rd'>Host cannot be empty!</h4>";
    credsOk = false;
  }

  if (credsOk == false) {
    $("#mqtt_connect_credentials_errors").html(errorList);
  } else {
    $("#mqtt_connect_credentials_errors").html("");
    connectMQTT();
  }
}
function connectMQTT() {
  Host = $("#host").val();
  pwd = $("#password").val();
  username = $("#username").val();
  $.ajax({
    url: "/MQTTConnect.json",
    dataType: "json",
    method: "POST",
    cache: false,
    headers: { host: Host, username: username, password: pwd },
    data: { timestamp: Date.now() },
  });

  startMQTConnectStatusInterval();
}

function startMQTConnectStatusInterval() {
  MQTTConnectInterval = setInterval(getWifiConnectStatus, 2800);
}

function stopWifiConnectStatusInterval() {
  if (MQTTConnectInterval != null) {
    clearInterval(MQTTConnectInterval);
    MQTTConnectInterval = null;
  }
}
