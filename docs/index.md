---
home: true
heroImage: logo.png
tagline: Documentation
actionText: Quick Start
actionTextIcon: fas fa-hand-point-right
actionLink: /guide/
subactionText: Download Release
subactionTextIcon: fas fa-download
subactionLink: https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/jobs/artifacts/wdissector/download?job=release

xfeatures:
- title: Wireless Fuzzing
  icon: fas fa-wifi
  details: Low-Level wireless fuzzing for Bluetooth Classic and 4G and 5G radio access networks. Specific hardware is required for fuzzing devices under test (DUT) over-the-air.
  
- title: Stateful Directed Fuzzing
  icon: fas fa-project-diagram
  details: WDissector fuzzer is feedback-driven and focuses in specific protocol states and packet fields during the fuzzing session for faster over-the-air results.

- title: Response Validation
  icon: fas fa-clipboard-check
  details: Validate device under test responses by specifying rules which can be applied to individual fields of a packet and the protocol state machine.

- title: Wireshark Integration
  icon: fas fa-network-wired
  details: Wireshark dissectors are used for the fuzzing engine and packet captures and reports are saved on wireshark pcapng format. Fuzzing actions and validation status comments are added to each packet for convinience.

- title: Realtime API
  icon: fas fa-clock
  details: A SocketIO server enables realtime connection with remote clients via WebSocket and allows headless operation for automated fuzzing sessions.

- title: Low-Latency UI
  icon: fas fa-history
  details: OpenGL based user interface (UI) for low-latency messages/packets visualization and Chromium Embedded (JavaScript) integration for advanced UI features.

footer: Made by ASSET Group in collaboration with Keysight Technologies.
---

<div class="home-link external" style="text-align: center">
  <a :href="$withBase('./old_greyhound/index.html')" target="_blank">
    GreyHound Documentation →
  </a>
</div>

<div class="features">
  <div class="feature" v-for="feat in $frontmatter.xfeatures">
      <h2><a v-bind:href="feat.link">
        <transition name="fade">
        <q-icon v-if="feat.icon" :name="feat.icon" style="font-size: 0.98em;" />
        </transition>
        {{ feat.title }}
      </a></h2>
      <p>{{ feat.details }}</p>
  </div>
</div>

<style>
.fade-enter-active, .fade-leave-active {
    transition: opacity 2.5s
}
.fade-enter, .fade-leave-to /* .fade-leave-active in <2.1.8 */ {
    opacity: 0
}
</style>
