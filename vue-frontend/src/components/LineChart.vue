<template>
  <div class="column is-12-mobile is-6-tablet is-5-desktop">
    <div class="card">
      <span class="dot" :class="{ active: activity }"></span>
      <div class="card-content">
        <div class="columns is-mobile is-vcentered">
          <div class="column is-narrow has-text-primary">
            <div class="card-icon pb-1 pt-2 px-2 has-background-link-light has-text-link">
              <svg width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
                <path d="M5 3v16h16v2H3V3h2zm15.293 3.293l1.414 1.414L16 13.414l-3-2.999-4.293 4.292-1.414-1.414L13 7.586l3 2.999 4.293-4.292z" />
              </svg>
            </div>
          </div>
          <div class="column">
            <h6 class="is-size-6 has-text-muted">{{ chart.name }}</h6>
          </div>
        </div>
        <div class="columns">
          <div class="column is-12">
            <line-chart :chart-data="chartData" :options="options" height="200px"></line-chart>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import LineChart from "./Charts/LineChart";

export default {
  props: ["chart"],

  components: {
    LineChart,
  },

  data() {
    return {
      activity: true,
      options: {
        responsive: true,
        aspectRatio: 1,
        height: 200,
        legend: {
          display: false,
        },
        scales: {
          xAxes: [
            {
              gridLines: {
                display: true,
              },
            },
          ],
        },
      },
    };
  },

  watch: {
    "chart.x_axis": {
      deep: true,
      handler() {
        if (this.activity === false) {
          this.activity = true;
          setTimeout(() => {
            this.activity = false;
          }, 100);
        }
      },
    },
    "chart.y_axis": {
      deep: true,
      handler() {
        if (this.activity === false) {
          this.activity = true;
          setTimeout(() => {
            this.activity = false;
          }, 100);
        }
      },
    },
  },

  computed: {
    chartData() {
      return {
        labels: this.chart.x_axis,
        datasets: [
          {
            label: "",
            backgroundColor: "#4c73f5",
            data: this.chart.y_axis,
            fill: false,
          },
        ],
      };
    },
  },

  mounted() {
    setTimeout(() => {
      this.activity = false;
    }, 500);
  },
};
</script>