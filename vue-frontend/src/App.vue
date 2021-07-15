<template>
  <div>
    <div class="section pt-1">
      <div class="container mt-6">
        <navbar />
      </div>
    </div>
    <div class="section pt-2">
      <transition name="fade" mode="out-in">
        <router-view
          :cards="cards"
          :charts="charts"
          :stats="stats"
          :log="log"
          :home="home"
        />
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
          value: 1304.0,
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
          animationRule: "quad",
          animationDuration: 1000,
          animatedValue: false,
          visible: false,
        },
        L: {
          value: 12.75,
          width: 300,
          height: 300,
          title: "L",
          fontTitleWeight: "bold",
          units: "mH",
          minValue: 0.05,
          maxValue: 12.75,
          majorTicks: this.getTicks(0.05, 12.75, 10),
          highlights: [],
          minorTicks: 20,
          strokeTicks: true,
          animation: true,
          animationRule: "quad",
          animationDuration: 1000,
          animatedValue: false,
          visible: false,
        },
        C2: {
          value: 1304.0,
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
          animationRule: "quad",
          animationDuration: 1000,
          animatedValue: false,
          visible: false,
        },
        swr: {
          value: 10.0,
          width: null,
          height: 150,
          title: false,
          units: false,
          minValue: 1.0,
          maxValue: 10.0,
          majorTicks: this.getTicks(1.0, 10.0, 5),
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
          animationRule: "quad",
          animationDuration: 1000,
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
          animationRule: "quad",
          animationDuration: 1000,
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
  },

  mounted() {
    Socket.$on("connected", () => {
      this.ws.connected = true;
      Socket.send(
        JSON.stringify({
          command: "getLayout",
        })
      );
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
          this.home.C1.minValue = json.actuator.C1.minPh;
          this.home.C1.maxValue = json.actuator.C1.maxPh;
          this.home.C1.majorTicks = this.getTicks(
            this.home.C1.minValue,
            this.home.C1.maxValue,
            8
          );
          this.home.C1.value = json.actuator.C1.maxPh;
          this.home.C1.visible = true;
        } else {
          this.home.C1.visible = false;
          this.home.C2.width = this.home.L.width;
          this.home.C2.height = this.home.L.height;
        }
        if (json.actuator.C2) {
          this.home.C2.minValue = json.actuator.C2.minPh;
          this.home.C2.maxValue = json.actuator.C2.maxPh;
          this.home.C2.majorTicks = this.getTicks(
            this.home.C2.minValue,
            this.home.C2.maxValue,
            8
          );
          this.home.C2.value = json.actuator.C2.maxPh;
          this.home.C2.visible = true;
        } else {
          this.home.C2.visible = false;
          this.home.C1.width = this.home.L.width;
          this.home.C1.height = this.home.L.height;
        }
        if (json.actuator.L) {
          this.home.L.minValue = json.actuator.L.minPh;
          this.home.L.maxValue = json.actuator.L.maxPh;
          this.home.L.majorTicks = this.getTicks(
            this.home.L.minValue,
            this.home.L.maxValue,
            10
          );
          this.home.L.value = json.actuator.L.maxPh;
          this.home.L.visible = true;
        } else {
          this.home.L.visible = false;
        }
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
        this.home.pwr.value = json.sensor.SWRMeterAds1115Ad8310.fwd || 0;
        this.home.swr.value = json.sensor.SWRMeterAds1115Ad8310.swr || 1;
        this.home.status.value = json.atu.state;
        let upTime = new Date(0);
        upTime.setSeconds(json.system.upTime || 0);
        this.stats.upTime =
          ((json.system.upTime || 0) / 60 / 60 / 24).toFixed(0) +
          " days " +
          upTime.toISOString().substr(11, 8);
        if (this.home.status.value === "ready") {
          this.home.status.symbol = "success";
        } else {
          this.home.status.symbol = "warning";
        }
      }
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