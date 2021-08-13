<template>
  <div class="column is-12-mobile is-4-tablet is-4-desktop is-3-fullhd">
    <div class="card">
      <span class="dot" :class="{ active: activity }"></span>
      <div class="card-content">
        <div class="columns is-mobile">
          <div class="column is-narrow"></div>
          <div class="column">
            <h6 class="is-size-6 has-text-muted">{{ card.name }}</h6>
            <h2 class="is-size-4 has-text-dark has-text-weight-semibold">
              {{ card.value }}
              <small class="is-size-6">{{ card.symbol }}</small>
            </h2>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  props: ["actuator"],

  data() {
    return {
      activity: true,
    };
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
