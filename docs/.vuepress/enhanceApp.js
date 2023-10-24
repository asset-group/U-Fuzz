/**
 * Client app enhancement file.
 *
 * https://v1.vuepress.vuejs.org/guide/basic-config.html#app-level-enhancements
 */

import VScrollin from 'vue-scrollin';
// Import quasar styles, they will be overriden by vuepress theme
import '@quasar/extras/material-icons/material-icons.css';
import '@quasar/extras/fontawesome-v5/fontawesome-v5.css';
import '@quasar/extras/roboto-font/roboto-font.css';
import 'quasar/dist/quasar.min.css';
import { isSSR } from 'quasar/src/plugins/Platform.js'
import Quasar from 'quasar';

export default ({
    Vue, // the version of Vue being used in the VuePress app
    options, // the options for the root Vue instance
    router, // the router instance for the app
    siteData // site metadata
}) => {
    // Register VScrollin
    Vue.component('VScrollin', VScrollin);
    if(!isSSR)
	{
		// Import quasar only on client
		Vue.use(Quasar);
	}
}
