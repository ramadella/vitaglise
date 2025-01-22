import { MQTTService } from "./mqttService.js";

var testimonials = document.getElementById('testimonials');
var control1 = document.getElementById('control1');
var control2 = document.getElementById('control2');
var control3 = document.getElementById('control3');


control1.onclick=function(){
    testimonials.style.transform = "translateX(870px)";
    control1.classList.add("active2");
    control2.classList.remove("active2");
    control3.classList.remove("active2");
}

control2.onclick=function(){  
    testimonials.style.transform = "translateX(0px)";
    control1.classList.remove("active2");
    control2.classList.add("active2");
    control3.classList.remove("active2");
}

control3.onclick=function(){
    testimonials.style.transform = "translateX(-870px)";
    control1.classList.remove("active2");
    control2.classList.remove("active2");
    control3.classList.add("active2");
}

var typed = new Typed(".text", {
  strings:["check your health", "education", "solution"],
      typeSpeed: 100,
      backSpeed: 100,
      backDelay: 1000,
      loop: true
});

// Target specific HTML items
const sideMenu = document.querySelector("aside");
const menuBtn = document.querySelector("#menu-btn");
const closeBtn = document.querySelector("#close-btn");
const themeToggler = document.querySelector(".theme-toggler");

// Holds the background color of all chart
var chartBGColor = getComputedStyle(document.body).getPropertyValue(
  "--chart-background"
);
var chartFontColor = getComputedStyle(document.body).getPropertyValue(
  "--chart-font-color"
);
var chartAxisColor = getComputedStyle(document.body).getPropertyValue(
  "--chart-axis-color"
);

/*
  Event listeners for any HTML click
*/
menuBtn.addEventListener("click", () => {
  sideMenu.style.display = "block";
});

closeBtn.addEventListener("click", () => {
  sideMenu.style.display = "none";
});

themeToggler.addEventListener("click", () => {
  document.body.classList.toggle("dark-theme-variables");
  themeToggler.querySelector("span:nth-child(1)").classList.toggle("active");
  themeToggler.querySelector("span:nth-child(2)").classList.toggle("active");

  // Update Chart background
  chartBGColor = getComputedStyle(document.body).getPropertyValue(
    "--chart-background"
  );
  chartFontColor = getComputedStyle(document.body).getPropertyValue(
    "--chart-font-color"
  );
  chartAxisColor = getComputedStyle(document.body).getPropertyValue(
    "--chart-axis-color"
  );
  updateChartsBackground();
});

/*
  Plotly.js graph and chart setup code
*/


var kolesterolHistoryDiv = document.getElementById("kolesterol-history");
var glukosaHistoryDiv = document.getElementById("glukosa-history");
var asamuratHistoryDiv = document.getElementById("asamurat-history");


const historyCharts = [
  kolesterolHistoryDiv,
  glukosaHistoryDiv,
  asamuratHistoryDiv,
];


// History Data
var kolesterolTrace = {
  x: [],
  y: [],
  name: "Kolesterol",
  mode: "lines+markers",
  type: "line",
  beginAtZero: true
};
var glukosaTrace = {
  x: [],
  y: [],
  name: "Glukosa",
  mode: "lines+markers",
  type: "line",
  beginAtZero: true
};
var asamuratTrace = {
  x: [],
  y: [],
  name: "Asam_Urat",
  mode: "lines+markers",
  type: "line",
  beginAtZero: true
};

var kolesterolLayout = {
  autosize: false,
  title: {
    text: "Kolesterol",
  },
  font: {
    size: 15,
    color: "rgb(175, 23, 0)",
    family: "Impact,Charcoal,sans-seri",
  },
  colorway: ["#D72600"],
  margin: { t: 40, b: 40, l: 50, r: 50, pad: 10 },
  plot_bgcolor: chartBGColor,
  paper_bgcolor: chartBGColor,
  xaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
    gridwidth: "2",
    autorange: true,
  },
  yaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
    gridwidth: "2",
    autorange: true,
  },
};
var glukosaLayout = {
  autosize: false,
  title: {
    text: "Glukosa",
  },
  font: {
    size: 15,
    color: "rgb(37, 136, 255)",
    family: "Impact,Charcoal,sans-seri",
  },
  colorway: ["#1C40F7"],
  margin: { t: 40, b: 40, l: 50, r: 50, pad: 0 },
  plot_bgcolor: chartBGColor,
  paper_bgcolor: chartBGColor,
  xaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
    gridwidth: "2",
  },
  yaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
  },
};
var asamuratLayout = {
  autosize: false,
  title: {
    text: "Asam Urat",
  },
  font: {
    size: 15,
    color: "#AAB116",
    family: "Impact,Charcoal,sans-seri",
  },
  colorway: ["#AAB116"],
  margin: { t: 40, b: 40, l: 50, r: 50, pad: 0 },
  plot_bgcolor: chartBGColor,
  paper_bgcolor: chartBGColor,
  xaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
    gridwidth: "2",
  },
  yaxis: {
    color: "rgb(0, 15, 117)",
    linecolor: "rgb(0, 115, 195)",
  },
};

var config = { responsive: true, displayModeBar: false };

// Event listener when page is loaded
window.addEventListener("load", (event) => {
  Plotly.newPlot(kolesterolHistoryDiv,[kolesterolTrace],kolesterolLayout,config);
  Plotly.newPlot(glukosaHistoryDiv, [glukosaTrace], glukosaLayout, config);
  Plotly.newPlot(asamuratHistoryDiv, [asamuratTrace], asamuratLayout, config);

  // Get MQTT Connection
  fetchMQTTConnection();

  // Run it initially
  handleDeviceChange(mediaQuery);
});

// Will hold the arrays we receive from our BME280 sensor
// Temperature
let newKolesterolXArray = [];
let newKolesterolYArray = [];
// Humidity
let newGlukosaXArray = [];
let newGlukosaYArray = [];
// Pressure
let newAsam_UratXArray = [];
let newAsam_UratYArray = [];

// The maximum number of data points displayed on our scatter/line graph
let MAX_GRAPH_POINTS = 12;
let ctr = 0;

// Callback function that will retrieve our latest sensor readings and redraw our Gauge with the latest readings
function updateSensorReadings(jsonResponse) {
  console.log(typeof jsonResponse);
  console.log(jsonResponse);

  let kolesterol = Number(jsonResponse.kolesterol).toFixed(2);
  let glukosa = Number(jsonResponse.glukosa).toFixed(2);
  let asamurat = Number(jsonResponse.asamurat).toFixed(2);

  updateBoxes(kolesterol, glukosa, asamurat);

  // Update kolesterol Line Chart
  updateCharts(
    kolesterolHistoryDiv,
    newKolesterolXArray,
    newKolesterolYArray,
    kolesterol
  );
  // Update glukosa Line Chart
  updateCharts(
    glukosaHistoryDiv,
    newGlukosaXArray,
    newGlukosaYArray,
    glukosa
  );
  // Update asam urat Line Chart
  updateCharts(
    asamuratHistoryDiv,
    newAsam_UratXArray,
    newAsam_UratYArray,
    asamurat
  );

  }
function updateBoxes(bpm, spo2) {
  let bpmDiv = document.getElementById("bpm");
  let spo2Div = document.getElementById("spo2");

  bpmDiv.innerHTML = bpm + " bpm";
  spo2Div.innerHTML = spo2 + " %";
}

function updateCharts(lineChartDiv, xArray, yArray, sensorRead) {
  if (xArray.length >= MAX_GRAPH_POINTS) {
    xArray.shift();
  }
  if (yArray.length >= MAX_GRAPH_POINTS) {
    yArray.shift();
  }
  xArray.push(ctr++);
  yArray.push(sensorRead);

  var data_update = {
    x: [xArray],
    y: [yArray],
  };

  Plotly.update(lineChartDiv, data_update);
}

function updateChartsBackground() {
  // updates the background color of historical charts
  var updateHistory = {
    plot_bgcolor: chartBGColor,
    paper_bgcolor: chartBGColor,
    font: {
      color: chartFontColor,
    },
    xaxis: {
      color: chartAxisColor,
      linecolor: chartAxisColor,
    },
    yaxis: {
      color: chartAxisColor,
      linecolor: chartAxisColor,
    },
  };
  historyCharts.forEach((chart) => Plotly.relayout(chart, updateHistory));

  // updates the background color of gauge charts
  var gaugeHistory = {
    plot_bgcolor: chartBGColor,
    paper_bgcolor: chartBGColor,
    font: {
      color: chartFontColor,
    },
    xaxis: {
      color: chartAxisColor,
      linecolor: chartAxisColor,
    },
    yaxis: {
      color: chartAxisColor,
      linecolor: chartAxisColor,
    },
  };
  gaugeCharts.forEach((chart) => Plotly.relayout(chart, gaugeHistory));
}

const mediaQuery = window.matchMedia("(max-width: 600px)");

mediaQuery.addEventListener("change", function (e) {
  handleDeviceChange(e);
});

function handleDeviceChange(e) {
  if (e.matches) {
    console.log("Inside Mobile");
    var updateHistory = {
      width: 323,
      height: 250,
      "xaxis.autorange": true,
      "yaxis.autorange": true,
    };
    historyCharts.forEach((chart) => Plotly.relayout(chart, updateHistory));
  } else {
    var updateHistory = {
      width: 550,
      height: 260,
      "xaxis.autorange": true,
      "yaxis.autorange": true,
    };
    historyCharts.forEach((chart) => Plotly.relayout(chart, updateHistory));
  }
}

/*
  MQTT Message Handling Code
*/
const mqttStatus = document.querySelector(".status");

function onConnect(message) {
  mqttStatus.textContent = "Connected";
}
function onMessage(topic, message) {
  var stringResponse = message.toString();
  var messageResponse = JSON.parse(stringResponse);
  updateSensorReadings(messageResponse);
}

function onError(error) {
  console.log(`Error encountered :: ${error}`);
  mqttStatus.textContent = "Error";
}

function onClose() {
  console.log(`MQTT connection closed!`);
  mqttStatus.textContent = "Closed";
}

function fetchMQTTConnection() {
  fetch("/mqttConnDetails", {
    method: "GET",
    headers: {
      "Content-type": "application/json; charset=UTF-8",
    },
  })
    .then(function (response) {
      return response.json();
    })
    .then(function (data) {
      initializeMQTTConnection(data.mqttServer, data.mqttTopic);
    })
    .catch((error) => console.error("Error getting MQTT Connection :", error));
}
function initializeMQTTConnection(mqttServer, mqttTopic) {
  console.log(
    `Initializing connection to :: ${mqttServer}, topic :: ${mqttTopic}`
  );
  var fnCallbacks = { onConnect, onMessage, onError, onClose };

  var mqttService = new MQTTService(mqttServer, fnCallbacks);
  mqttService.connect();

  mqttService.subscribe(mqttTopic);
}