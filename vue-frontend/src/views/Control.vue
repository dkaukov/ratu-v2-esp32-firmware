<template>
  <div>
    <div class="container" v-if="true">
      <div class="section">
        <div class="columns is-vcentered">
          <div class="column is-full has-text-centered">
            <linear-gauge :value="home.swr.value" :options="home.swr" ref="swr"></linear-gauge>
            <linear-gauge :value="home.pwr.value" :options="home.pwr" ref="pwr" style="margin-top: -118px"></linear-gauge>
          </div>
        </div>
        <div class="columns is-vcentered">
          <div class="column is-1">
            <h6>{{ home.C1.title }}</h6>
          </div>
          <div class="column has-text-centered">
            <input class="slider is-fullwidth s-circle is-large" :min="home.C1.minValue" :max="home.C1.maxValue" v-model="home.C1.value" type="range" />
          </div>
          <div class="column is-1">
            <button class="button is-primary is-light is-outlined">
              {{ home.C1.value }}
            </button>
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
//import GenericCard from '@/components/GenericCard.vue';
//import TemperatureCard from '@/components/TemperatureCard.vue';
//import HumdidityCard from '@/components/HumidityCard.vue';
//import StatusCard from "@/components/StatusCard.vue";
//import ProgressCard from '@/components/ProgressCard.vue';
//import SliderCard from '@/components/SliderCard.vue';
//import ButtonCard from '@/components/ButtonCard.vue';

//import BarChart from '@/components/BarChart.vue';
//import LineChart from '@/components/LineChart.vue';

import LinearGauge from "vue-canvas-gauges/src/LinearGauge";
//import RadialGauge from "vue-canvas-gauges/src/RadialGauge";

export default {
  name: "control",

  props: ["cards", "charts", "home"],

  components: {
    //GenericCard,
    //TemperatureCard,
    //HumdidityCard,
    //StatusCard,
    //ProgressCard,
    //ButtonCard,
    //SliderCard,
    //BarChart,
    LinearGauge,
    //RadialGauge,
    //LineChart
  },

  data() {
    return {
      chart: {
        id: "",
        type: "line",
        name: "name",
        x_axis: [1, 2, 3, 4, 5],
        y_axis: [1, 2, 3, 4, 5],
      },
    };
  },

  methods: {},

  mounted() {
    this.$refs.c1.$watch("value", function (newVal, oldVal) {
      if (newVal !== oldVal) {
        this.chart._value = newVal;
      }
    });
    this.$refs.c2.$watch("value", function (newVal, oldVal) {
      if (newVal !== oldVal) {
        this.chart._value = newVal;
      }
    });
    this.$refs.l.$watch("value", function (newVal, oldVal) {
      if (newVal !== oldVal) {
        this.chart._value = newVal;
      }
    });
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
};
</script>