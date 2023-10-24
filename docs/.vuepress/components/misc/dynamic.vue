<template>
  <span class="componentWrapper">
    <component :is="componentLoader" v-bind="$attrs"> </component>
  </span>
</template>
<script>
export default {
  name: "misc-dynamic",
  props: {
    component: {
      type: String,
      required: true,
      default: () => null,
    },
    global: {
      type: Boolean,
      required: false,
      default: () => false,
    },
  },
  computed: {
    componentLoader() {
      if(!this.global)
        return () => import("../" + this.component);
      else 
        return this.component;
    },
  },
};
</script>
