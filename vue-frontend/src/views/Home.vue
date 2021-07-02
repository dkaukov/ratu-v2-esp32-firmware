<template>
  <div>
    <div class="container" v-if="true">
      <div class="section">
        <div class="row">
            <div class="columns" style="font-weight: 400; font-size: 18px">
              <div class="column">
              </div>
              <div class="column" style="is-gapless">
                  <linear-gauge :value="1.1" :options="swr"></linear-gauge>
                  <linear-gauge :value="10" :options="pwr" style="margin-top: -118px;"></linear-gauge>
              </div>
              <div class="column">
              </div>
            </div>
        </div>
        <div class="row">
          <div class="columns" style="font-weight: 400; font-size: 18px">
            <div class="column">
              <radial-gauge :options="C1" :value="50" ></radial-gauge>
            </div>
            <div class="column">
              <radial-gauge :options="L"></radial-gauge>
            </div>
            <div class="column">
              <radial-gauge :options="C2"></radial-gauge>
            </div>
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
              <svg width="24px" height="24px" class="spinner" style="vertical-align: middle;" viewBox="0 0 24 24" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
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
import GenericCard from '@/components/GenericCard.vue';
import TemperatureCard from '@/components/TemperatureCard.vue';
import HumdidityCard from '@/components/HumidityCard.vue';
import StatusCard from '@/components/StatusCard.vue';
import ProgressCard from '@/components/ProgressCard.vue';
import SliderCard from '@/components/SliderCard.vue';
import ButtonCard from '@/components/ButtonCard.vue';

import BarChart from '@/components/BarChart.vue';
import LineChart from '@/components/LineChart.vue';

import LinearGauge from 'vue-canvas-gauges/src/LinearGauge'
import RadialGauge from 'vue-canvas-gauges/src/RadialGauge'

export default {
  name: 'home',

  props: ['cards', 'charts'],

  components: {
    GenericCard,
    TemperatureCard,
    HumdidityCard,
    StatusCard,
    ProgressCard,
    ButtonCard,
    SliderCard,
    BarChart,
    LinearGauge,
    RadialGauge,
    LineChart
  },

  data() {
    return {
      C1: {
        value: 50, 
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
        animationRule: 'bounce'
      },
      L: {
        value: 233, 
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
        animationRule: 'bounce'
      },
      C2: {
        value: 233, 
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
        animationRule: 'bounce'
      },
      swr: {
        width: 600, 
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
        animationRule: 'bounce',
        colorPlate: "rgba(0,0,0,0)"
      },
      pwr: {
        width: 600, 
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
        animationRule: 'bounce',
        colorPlate: "rgba(0,0,0,0)"
      },
      status: {
        symbol: "success",
        value: "Idle"
      },
      chart: {
        id: "",
        type: "line",
        name: "name",
        x_axis: [1,2,3,4,5],
        y_axis: [1,2,3,4,5], 
      },
    }
  },

  methods: {
    getParticularCards(type) {
      let cards = [];
      for(let card of this.cards){
        if(card.type === type){
          cards.push(card);
        }
      }
      return cards;
    },
    getParticularCharts(type) {
      let charts = [];
      for(let chart of this.charts){
        if(chart.type === type){
          charts.push(chart);
        }
      }
      return charts;
    },
    getTicks(min, max, count) {
      let res = [];
      for (let i = 0; i <= count; i++) {
        res.push(Number(min + (max - min) / count * i).toFixed(2));
      }
      return res;
    }
  }
}
</script>