<template>
  <div>
    <div class="container" v-if="true">
      <div class="section">
        <div class="columns is-vcentered">
          <div class="column is-four-fifths has-text-centered">
            <linear-gauge :value="control.swr.value" :options="control.swr" ref="swr"></linear-gauge>
            <linear-gauge :value="control.pwr.value" :options="control.pwr" ref="pwr" style="margin-top: -118px"></linear-gauge>
          </div>
          <div class="column is-1 has-text-centered">
            <button class="button is-primary" style="cursor: pointer" @click="sendTuneL" :disabled="this.tuneDisabled">Tune L</button>
          </div>
          <div class="column is-1 has-text-centered">
            <button class="button is-primary" style="cursor: pointer" @click="sendTuneC" :disabled="this.tuneDisabled">Tune C</button>
          </div>
        </div>
        <div class="card-content">
          <div class="columns ml-2">
            <actuator-card class="ml-2" :key="'card_c1'" :actuator="control.C1" v-if="control.C1.visible"></actuator-card>
            <actuator-card class="ml-2" :key="'card_l'" :actuator="control.L" v-if="control.L.visible"></actuator-card>
            <actuator-card class="ml-2" :key="'card_c2'" :actuator="control.C2" v-if="control.C2.visible"></actuator-card>
            <div class="column"></div>
          </div>
        </div>
      </div>
    </div>
    <div class="container" v-else>
      <div class="section">
        <div class="row">
          <div class="columns is-centered has-text-centered" style="font-weight: 400; font-size: 18px">
            <div class="column">
              Loading
              <svg width="24px" height="24px" class="spinner" style="vertical-align: middle" viewBox="0 0 24 24" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
                <g id="Stockholm-icons-/-Code-/-Loading" stroke="none" stroke-width="1" fill="none" fill-rule="evenodd">
                  <g id="Group">
                    <polygon id="Shape" points="0 0 24 0 24 24 0 24"></polygon>
                  </g>
                  <path d="M12,4 L12,6 C8.6862915,6 6,8.6862915 6,12 C6,15.3137085 8.6862915,18 12,18 C15.3137085,18 18,15.3137085 18,12 C18,10.9603196 17.7360885,9.96126435 17.2402578,9.07513926 L18.9856052,8.09853149 C19.6473536,9.28117708 20,10.6161442 20,12 C20,16.418278 16.418278,20 12,20 C7.581722,20 4,16.418278 4,12 C4,7.581722 7.581722,4 12,4 Z" id="Oval-3" fill="currentColor" opacity="0.3" transform="translate(12.000000, 12.000000) scale(-1, 1) translate(-12.000000, -12.000000) "></path>
                </g>
              </svg>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import LinearGauge from "vue-canvas-gauges/src/LinearGauge";
import ActuatorCard from "@/components/ActuatorCard.vue";
import EventBus from "@/event-bus.js";

export default {
  name: "control",

  props: ["cards", "charts", "control"],

  components: {
    LinearGauge,
    ActuatorCard,
  },

  data() {
    return {
      tuneDisabled: !(this.control.status.value === "ready"),
    };
  },

  methods: {
    sendTuneL() {
      this.msg = {
        config: {
          atu: {
            tuningMode: "ATU_TUNING_TYPE_L",
          },
        },
      };
      this.tuneDisabled = true;
      EventBus.$emit("tune", this.msg);
    },
    sendTuneC() {
      this.msg = {
        config: {
          atu: {
            tuningMode: "ATU_TUNING_TYPE_C",
          },
        },
      };
      this.tuneDisabled = true;
      EventBus.$emit("tune", this.msg);
    },
  },

  mounted() {
    this.$refs.swr.$watch("value", function (newVal, oldVal) {
      if (newVal !== oldVal) {
        this.chart._value = newVal;
      }
    });
    this.$refs.pwr.$watch("value", function (newVal, oldVal) {
      if (newVal !== oldVal) {
        this.chart._value = newVal;
      }
    });
  },

  watch: {
    "control.status.value": function () {
      this.tuneDisabled = !(this.control.status.value === "ready");
    },
  },
};
</script>