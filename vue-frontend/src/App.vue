<template>
  <div>
    <div class="section pt-1">
      <div class="container mt-6">
        <navbar />
      </div>
    </div>
    <div class="section pt-2">
      <transition name="fade" mode="out-in">
        <router-view :cards="cards" :charts="charts" :stats="stats" :log="log" :home="home" :control="control" />
      </transition>
    </div>
  </div>
</template>

<script>
import EventBus from "./event-bus.js";
import Socket from "./socket";
import Navbar from "./components/Navbar";

export default {
  components: {
    Navbar,
  },

  data() {
    return {
      ws: {
        url: "",
        connected: false,
      },
      stats: {
        enabled: false,
        releaseTag: null,
        sdk: null,
        chipId: null,
        sketchHash: null,
        macAddress: null,
        freeHeap: null,
        minFreeHeap: null,
        maxAllocHeap: null,
        stackHighWaterMark: null,
        wifiMode: null,
        hostname: "",
        wifiSignal: null,
        upTime: null,
        hardware: null,
      },
      log: {
        lines: "",
      },
      home: {
        C1: {
          id: "C1",
          value: 1304.0,
          rawValue: 255,
          width: 270,
          height: 270,
          title: "C\u2081",
          fontTitleWeight: "bold",
          units: "\u338a",
          minValue: 5.0,
          maxValue: 1304.0,
          majorTicks: this.getTicks(5.0, 1304.0, 8),
          highlights: [],
          minorTicks: 20,
          strokeTicks: true,
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        L: {
          id: "L",
          value: 12.75,
          rawValue: 255,
          width: 300,
          height: 300,
          title: "L",
          fontTitleWeight: "bold",
          units: "μH",
          minValue: 0.05,
          maxValue: 12.75,
          majorTicks: this.getTicks(0.05, 12.75, 10),
          highlights: [],
          minorTicks: 20,
          strokeTicks: true,
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        C2: {
          id: "C2",
          value: 1304.0,
          rawValue: 255,
          width: 270,
          height: 270,
          title: "C\u2082",
          fontTitleWeight: "bold",
          units: "\u338a",
          minValue: 5.0,
          maxValue: 1304.0,
          majorTicks: this.getTicks(5.0, 1304.0, 8),
          highlights: [],
          minorTicks: 20,
          strokeTicks: true,
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        swr: {
          value: 3.0,
          width: null,
          height: 150,
          title: false,
          units: false,
          minValue: 1.0,
          maxValue: 3.0,
          majorTicks: this.getTicks(1.0, 3.0, 5),
          highlights: [],
          //minorTicks: 20,
          strokeTicks: true,
          barBeginCircle: false,
          needleSide: "left",
          tickSide: "left",
          numberSide: "left",
          borders: false,
          borderShadowWidth: 0,
          needleType: "line",
          needleWidth: 2,
          barWidth: 5,
          valueBox: true,
          colorPlate: "rgba(0,0,0,0)",
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
        },
        pwr: {
          value: 100.0,
          width: null,
          height: 150,
          title: false,
          units: false,
          minValue: 0,
          maxValue: 100.0,
          majorTicks: this.getTicks(0, 100.0, 10),
          highlights: [],
          //minorTicks: 20,
          strokeTicks: true,
          barBeginCircle: false,
          needleSide: "right",
          tickSide: "right",
          numberSide: "right",
          borders: false,
          borderShadowWidth: 0,
          needleType: "line",
          needleWidth: 2,
          barWidth: 5,
          valueBox: true,
          colorPlate: "rgba(0,0,0,0)",
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
        },
        status: {
          symbol: "danger",
          name: "ATU status",
          value: "Booting up",
        },
      },
      control: {
        C1: {
          id: "C1",
          value: 1304.0,
          rawValue: 255,
          title: "C\u2081",
          units: "\u338a",
          minValue: 5.0,
          maxValue: 1304.0,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        L: {
          id: "L",
          value: 12.75,
          rawValue: 255,
          title: "L",
          units: "μH",
          minValue: 0.05,
          maxValue: 12.75,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        C2: {
          id: "C2",
          value: 1304.0,
          rawValue: 255,
          title: "C\u2082",
          units: "\u338a",
          minValue: 5.0,
          maxValue: 1304.0,
          visible: false,
          step: 1,
          isReady: false,
          isInverted: false,
        },
        swr: {
          value: 3.0,
          width: null,
          height: 150,
          title: false,
          units: false,
          minValue: 1.0,
          maxValue: 3.0,
          majorTicks: this.getTicks(1.0, 3.0, 5),
          highlights: [],
          //minorTicks: 20,
          strokeTicks: true,
          barBeginCircle: false,
          needleSide: "left",
          tickSide: "left",
          numberSide: "left",
          borders: false,
          borderShadowWidth: 0,
          needleType: "line",
          needleWidth: 2,
          barWidth: 5,
          valueBox: true,
          colorPlate: "rgba(0,0,0,0)",
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
        },
        pwr: {
          value: 100.0,
          width: null,
          height: 150,
          title: false,
          units: false,
          minValue: 0,
          maxValue: 100.0,
          majorTicks: this.getTicks(0, 100.0, 10),
          highlights: [],
          //minorTicks: 20,
          strokeTicks: true,
          barBeginCircle: false,
          needleSide: "right",
          tickSide: "right",
          numberSide: "right",
          borders: false,
          borderShadowWidth: 0,
          needleType: "line",
          needleWidth: 2,
          barWidth: 5,
          valueBox: true,
          colorPlate: "rgba(0,0,0,0)",
          animation: true,
          animationRule: "linear",
          animationDuration: 200,
          animatedValue: false,
        },
        status: {
          symbol: "danger",
          name: "ATU status",
          value: "Booting up",
        },
      },
      cards: [],
      charts: [],
    };
  },

  methods: {
    getTicks(min, max, count) {
      let res = [];
      for (let i = 0; i <= count; i++) {
        res.push(Number(min + ((max - min) / count) * i).toFixed(2));
      }
      return res;
    },
    constrain(val, min, max) {
      if (val < min) {
        return min;
      }
      if (val > max) {
        return max;
      }
      return val;
    },
  },

  mounted() {
    Socket.$on("connected", () => {
      this.ws.connected = true;
    });

    Socket.$on("disconnected", () => {
      this.ws.connected = false;
    });

    Socket.$on("message", (json) => {
      this.ws.connected = true;

      if (json.topic === "log") {
        EventBus.$emit("logLine", json.message);
        this.log.lines = this.log.lines + "\n" + json.message;
      }

      if (json.topic === "config") {
        EventBus.$emit("config", json);
        this.stats.enabled = true;
        this.stats.sdk = json.device.sdkVersion;
        this.stats.releaseTag = json.device.sketchMD5;
        this.stats.chipId = json.device.chipModel;
        this.stats.sketchHash = json.device.sketchMD5;
        this.stats.macAddress = json.device.wifi.macAddress;
        this.stats.wifiMode = 1;
        this.stats.hostname = json.device.wifi.hostname;
        this.stats.hardware = json.device.hardware;
        if (json.actuator.C1) {
          this.home.C1.minValue = Math.min(json.actuator.C1.minPh, json.actuator.C1.maxPh);
          this.home.C1.maxValue = Math.max(json.actuator.C1.minPh, json.actuator.C1.maxPh);
          this.home.C1.isInverted = json.actuator.C1.minPh > json.actuator.C1.maxPh;
          this.home.C1.majorTicks = this.getTicks(this.home.C1.minValue, this.home.C1.maxValue, 8);
          this.home.C1.value = json.actuator.C1.maxPh;
          this.home.C1.visible = true;
          if (Math.abs(json.actuator.C1.max - json.actuator.C1.min) > 1000) {
            this.home.C1.step = 10;
          }
          if (this.home.C1.isInverted) {
            this.home.C1.step = -this.home.C1.step;
          }
        } else {
          this.home.C1.visible = false;
          this.home.C2.width = this.home.L.width;
          this.home.C2.height = this.home.L.height;
        }
        if (json.actuator.C2) {
          this.home.C2.minValue = Math.min(json.actuator.C2.minPh, json.actuator.C2.maxPh);
          this.home.C2.maxValue = Math.max(json.actuator.C2.minPh, json.actuator.C2.maxPh);
          this.home.C2.isInverted = json.actuator.C2.minPh > json.actuator.C2.maxPh;
          this.home.C2.majorTicks = this.getTicks(this.home.C2.minValue, this.home.C2.maxValue, 8);
          this.home.C2.value = json.actuator.C2.maxPh;
          this.home.C2.visible = true;
          if (Math.abs(json.actuator.C2.max - json.actuator.C2.min) > 1000) {
            this.home.C2.step = 10;
          }
          if (this.home.C2.isInverted) {
            this.home.C2.step = -this.home.C2.step;
          }
        } else {
          this.home.C2.visible = false;
          this.home.C1.width = this.home.L.width;
          this.home.C1.height = this.home.L.height;
        }
        if (json.actuator.L) {
          this.home.L.minValue = Math.min(json.actuator.L.minPh, json.actuator.L.maxPh);
          this.home.L.maxValue = Math.max(json.actuator.L.minPh, json.actuator.L.maxPh);
          this.home.L.isInverted = json.actuator.L.minPh > json.actuator.L.maxPh;
          this.home.L.majorTicks = this.getTicks(this.home.L.minValue, this.home.L.maxValue, 10);
          this.home.L.value = json.actuator.L.maxPh;
          this.home.L.visible = true;
          if (Math.abs(json.actuator.L.max - json.actuator.L.min > 1000)) {
            this.home.L.step = 10;
          }
          if (this.home.L.isInverted) {
            this.home.L.step = -this.home.L.step;
          }
        } else {
          this.home.L.visible = false;
        }
        this.control.C1.minValue = this.home.C1.minValue;
        this.control.C1.maxValue = this.home.C1.maxValue;
        this.control.C1.isInverted = this.home.C1.isInverted;
        this.control.C1.majorTicks = this.home.C1.majorTicks;
        this.control.C1.value = this.home.C1.value;
        this.control.C1.step = this.home.C1.step;
        this.control.C1.visible = this.home.C1.visible;
        this.control.C2.minValue = this.home.C2.minValue;
        this.control.C2.maxValue = this.home.C2.maxValue;
        this.control.C2.isInverted = this.home.C2.isInverted;
        this.control.C2.majorTicks = this.home.C2.majorTicks;
        this.control.C1.value = this.home.C2.value;
        this.control.C2.visible = this.home.C2.visible;
        this.control.C2.step = this.home.C2.step;
        this.control.L.minValue = this.home.L.minValue;
        this.control.L.maxValue = this.home.L.maxValue;
        this.control.L.isInverted = this.home.L.isInverted;
        this.control.L.majorTicks = this.home.L.majorTicks;
        this.control.L.value = this.home.L.value;
        this.control.L.visible = this.home.L.visible;
        this.control.L.step = this.home.L.step;
      }

      if (json.topic === "status") {
        EventBus.$emit("status", json);
        this.stats.freeHeap = json.system.freeHeap;
        this.stats.minFreeHeap = json.system.minFreeHeap;
        this.stats.maxAllocHeap = json.system.maxAllocHeap;
        this.stats.stackHighWaterMark = json.system.stackHighWaterMark;
        this.stats.wifiSignal = json.system.rssi;
        this.home.C1.value = (json.actuator.C1 || {}).phValue;
        this.home.C2.value = (json.actuator.C2 || {}).phValue;
        this.home.L.value = (json.actuator.L || {}).phValue;
        this.control.C1.value = (json.actuator.C1 || {}).phValue;
        this.control.C2.value = (json.actuator.C2 || {}).phValue;
        this.control.L.value = (json.actuator.L || {}).phValue;
        this.home.C1.rawValue = (json.actuator.C1 || {}).value;
        this.home.C2.rawValue = (json.actuator.C2 || {}).value;
        this.home.L.rawValue = (json.actuator.L || {}).value;
        this.control.C1.rawValue = (json.actuator.C1 || {}).value;
        this.control.C2.rawValue = (json.actuator.C2 || {}).value;
        this.control.L.rawValue = (json.actuator.L || {}).value;
        this.home.C1.isReady = (json.actuator.C1 || {}).isReady;
        this.home.C2.isReady = (json.actuator.C2 || {}).isReady;
        this.home.L.isReady = (json.actuator.L || {}).isReady;
        this.control.C1.isReady = (json.actuator.C1 || {}).isReady;
        this.control.C2.isReady = (json.actuator.C2 || {}).isReady;
        this.control.L.isReady = (json.actuator.L || {}).isReady;
        this.home.pwr.value = this.constrain(json.sensor.SWRMeterAds1115Ad8310.fwd || 0, this.home.pwr.minValue, this.home.pwr.maxValue);
        this.home.swr.value = this.constrain(json.sensor.SWRMeterAds1115Ad8310.swr || 1, this.home.swr.minValue, this.home.swr.maxValue);
        this.control.pwr.value = this.constrain(json.sensor.SWRMeterAds1115Ad8310.fwd || 0, this.control.pwr.minValue, this.control.pwr.maxValue);
        this.control.swr.value = this.constrain(json.sensor.SWRMeterAds1115Ad8310.swr || 1, this.control.swr.minValue, this.control.swr.maxValue);
        this.home.status.value = json.atu.state;
        let upTime = new Date(0);
        upTime.setSeconds(json.system.upTime || 0);
        this.stats.upTime = ((json.system.upTime || 0) / 60 / 60 / 24).toFixed(0) + " days " + upTime.toISOString().substr(11, 8);
        if (this.home.status.value === "ready") {
          this.home.status.symbol = "success";
        } else {
          this.home.status.symbol = "warning";
        }
        this.control.status.value = this.home.status.value;
        this.control.status.symbol = this.home.status.symbol;
      }
    });

    EventBus.$on("actuate", (data) => {
      Socket.send(
        JSON.stringify({
          command: "actuate",
          actuator: data.actuator,
        })
      );
    });

    EventBus.$on("tune", (data) => {
      Socket.send(
        JSON.stringify({
          command: "tune",
          config: data.config,
        })
      );
    });

    EventBus.$on("buttonClicked", (data) => {
      Socket.send(
        JSON.stringify({
          command: "buttonClicked",
          id: data.id,
          value: data.value,
        })
      );
    });

    EventBus.$on("sliderChanged", (msg) => {
      Socket.send(
        JSON.stringify({
          command: "sliderChanged",
          id: msg.id,
          value: msg.value,
        })
      );
    });
  },
};
</script>

<style lang="scss">
@import "./assets/scss/main.scss";
</style>