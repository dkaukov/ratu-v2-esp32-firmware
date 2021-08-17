<template>
  <div class="column box">
    <h4 class="title is-4 is-pulled-right">{{ actuator.title }}</h4>
    <v-sevenseg :value="round(actuator.value)" color-back="transparent" color-on="green" color-off="rgb(255, 240, 255)" height="40" digits="6" slant="10"></v-sevenseg>
    <div class="block" />
    <div>
      <input class="slider is-fullwidth s-circle is-large" :min="0" :max="100" type="range" style="opacity: 0.7" />
      <progress class="progress is-small is-primary is-fullwidth" :min="actuator.minValue" :max="actuator.maxValue" :value="actuator.value" style="margin-top: -22px; height: 11px" />
    </div>
    <div class="block" />
    <div class="buttons are-small is-pulled-right">
      <button class="button is-primary is-light">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24">
          <path fill="none" d="M0 0h24v24H0z" />
          <path d="M21 20a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1V9.49a1 1 0 0 1 .386-.79l8-6.222a1 1 0 0 1 1.228 0l8 6.222a1 1 0 0 1 .386.79V20zm-2-1V9.978l-7-5.444-7 5.444V19h14z" />
        </svg>
      </button>
      <button class="button is-primary is-light">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24">
          <path fill="none" d="M0 0h24v24H0z" />
          <path d="M12 10.667l9.223-6.149a.5.5 0 0 1 .777.416v14.132a.5.5 0 0 1-.777.416L12 13.333v5.733a.5.5 0 0 1-.777.416L.624 12.416a.5.5 0 0 1 0-.832l10.599-7.066a.5.5 0 0 1 .777.416v5.733zm-2 5.596V7.737L3.606 12 10 16.263zm10 0V7.737L13.606 12 20 16.263z" />
        </svg>
      </button>
      <button class="button is-primary is-light">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="24" height="24">
          <path fill="none" d="M0 0h24v24H0z" />
          <path d="M12 13.333l-9.223 6.149A.5.5 0 0 1 2 19.066V4.934a.5.5 0 0 1 .777-.416L12 10.667V4.934a.5.5 0 0 1 .777-.416l10.599 7.066a.5.5 0 0 1 0 .832l-10.599 7.066a.5.5 0 0 1-.777-.416v-5.733zM10.394 12L4 7.737v8.526L10.394 12zM14 7.737v8.526L20.394 12 14 7.737z" />
        </svg>
      </button>
    </div>
  </div>
</template>

<script>
import VSevenseg from "v-sevenseg/src/components/VSevenseg.vue";

export default {
  props: ["actuator"],

  components: {
    VSevenseg,
  },

  data() {
    return {
      activity: true,
    };
  },

  methods: {
    round(val) {
      return val.toFixed(2);
    },
  },

  watch: {
    "actuator.value": function () {
      this.activity = true;
      setTimeout(() => {
        this.activity = false;
      }, 100);
    },
  },

  mounted() {
    setTimeout(() => {
      this.activity = false;
    }, 500);
  },
};
</script>
