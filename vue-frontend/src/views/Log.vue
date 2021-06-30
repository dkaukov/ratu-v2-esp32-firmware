<template>
  <div class="container" >
    <log-viewer :log="log" />
  </div>
</template>

<script>
import LogViewer from "@femessage/log-viewer";
import EventBus from "@/event-bus.js";

export default {
  name: "log",

  components: {
    LogViewer,
  },

  data() {
    return {
      log: "",
    };
  },

  methods: {},

  mounted() {
    EventBus.$on("logLine", (data) => {
      this.log = this.log + "\n" + data;
    });
  },
};
</script>