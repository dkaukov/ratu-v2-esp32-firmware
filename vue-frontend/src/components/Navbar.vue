<template>
  <div class="card">
    <div class="card-content">
      <nav class="navbar" role="navigation" aria-label="main navigation">
        <div class="navbar-brand">
          <router-link class="navbar-item" to="/">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="48" height="48">
              <path fill="none" d="M0 0h24v24H0z" />
              <path d="M12.506 3.623l-1.023 1.772c-2.91-.879-5.514-.45-6.411 1.105-1.178 2.04.79 5.652 4.678 7.897s8 2.142 9.178.103c.898-1.555-.033-4.024-2.249-6.105l1.023-1.772c3.082 2.709 4.463 6.27 2.958 8.877-1.86 3.222-7.189 3.355-11.91.63C4.029 13.402 1.48 8.721 3.34 5.5c1.505-2.607 5.28-3.192 9.166-1.877zm3.378-1.85l1.732 1-5 8.66-1.732-1 5-8.66zM6.732 20H17v2H5.017a.995.995 0 0 1-.883-.5 1.005 1.005 0 0 1 0-1l2.25-3.897 1.732 1L6.732 20z" />
            </svg>
          </router-link>

          <a role="button" class="navbar-burger burger" aria-label="menu" :aria-expanded="open" :class="{ 'is-active': open }" @click.prevent="open = !open">
            <span aria-hidden="true"></span>
            <span aria-hidden="true"></span>
            <span aria-hidden="true"></span>
          </a>
        </div>

        <div class="navbar-menu" :class="{ 'is-active': open }">
          <div class="navbar-end">
            <router-link class="navbar-item" @click.native="open = false" to="/" exact> Overview </router-link>
            <router-link class="navbar-item" @click.native="open = false" to="/control" exact> Control </router-link>
            <router-link class="navbar-item" @click.native="open = false" to="/log" exact> Log </router-link>
            <router-link class="navbar-item" @click.native="open = false" to="/statistics"> Statistics </router-link>
          </div>
        </div>
      </nav>
    </div>
  </div>
</template>

<script>
export default {
  props: ["connected", "stats", "log", "control"],

  data() {
    return {
      open: false,
    };
  },
};
</script>
