<template>
  <div class="container" >
    <log-viewer :log="log.lines" :hasNumber="false"  />
  </div>
</template>

<script>
import LogViewer from "@femessage/log-viewer";

export default {
  name: "log",

  components: {
    LogViewer,
  },

  methods: {},

  props: ['log'],

  mounted() {},
};
</script>

<style lang="less">
.log-viewer {
  font-size: 15px !important;
  font-family: monospace !important;
}
</style>
