const { description } = require('../../package')

var sideBarChildren = [
    '', /* index.md */
    'guide',
    'stacks',
    'fuzzers',
    'drivers',
    'api',
    'exploits',
    'modules',
    'architecture',
    'tutorials',
    'licenses',
    'todo'
]

module.exports = {
    /**
     * Ref：https://v1.vuepress.vuejs.org/config/#title
     */
    title: 'WDissector Fuzzer',
    dest: './release/docs',
    base: '' || process.env['VUEPRESS_BASE_URL'],

    /**
     * Ref：https://v1.vuepress.vuejs.org/config/#description
     */
    description: description,

    /**
     * Extra tags to be injected to the page HTML `<head>`
     *
     * ref：https://v1.vuepress.vuejs.org/config/#head
     */
    head: [
        ['meta', { name: 'theme-color', content: '#3eaf7c' }],
        ['meta', { name: 'apple-mobile-web-app-capable', content: 'yes' }],
        ['meta', { name: 'apple-mobile-web-app-status-bar-style', content: 'black' }]
    ],

    patterns: process.argv.includes("export") ? ['**/*.md', '**/*.vue', '!papers/**', '!old_greyhound/**', '!index.md', '!todo.md'] : ['**/*.md', '**/*.vue', '!papers/**', '!old_greyhound/**'],

    /**
     * Theme configuration, here is the default theme configuration for VuePress.
     *
     * ref：https://v1.vuepress.vuejs.org/theme/default-theme-config.html
     */
    themeConfig: {
        logo: './sutd.png',
        repo: 'https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/tree/wdissector',
        repoLabel: 'Repository',
        editLinks: false,
        docsDir: 'docs',
        editLinkText: '',
        lastUpdated: true,
        nav: [{
                text: 'Getting Started',
                link: '/guide',
                icon: 'fas fa-hand-point-right'
            },
            {
                text: 'Topics',
                icon: 'fas fa-book',
                items: [
                    { text: 'Protocol Stacks', link: '/stacks', icon: 'fas fa-layer-group' },
                    { text: 'Fuzzers', link: '/fuzzers', icon: 'fas fa-random' },
                    { text: 'Drivers', link: '/drivers', icon: 'fas fa-microchip' },
                    { text: 'Web API', link: '/api', icon: 'fas fa-cloud' },
                    { text: 'Exploits', link: '/exploits', icon: 'fas fa-radiation' },
                    { text: 'Modules', link: '/modules', icon: 'fas fa-cube' },
                    { text: 'Architecture', link: '/architecture', icon: 'fas fa-drafting-compass' },
                    { text: 'Tutorials', link: '/tutorials', icon: 'fas fa-graduation-cap' },
                    { text: 'Licenses', link: '/licenses', icon: 'fas fa-copyright' },
                    { text: 'Todo', link: '/todo', icon: 'fas fa-tasks' },
                ]
            },
            {
                text: 'GreyHound',
                link: './old_greyhound/index.html',
                target: '_blank'
            },
        ],
        sidebar: {
            '/': [{
                title: 'Topics',
                initialOpenGroupIndex: 0,
                sidebarDepth: 2,
                collapsable: false,
                children: sideBarChildren
            }],
        }
    },

    /**
     * Apply plugins，ref：https://v1.vuepress.vuejs.org/plugin/
     */
    plugins: [
        '@vuepress/plugin-back-to-top',
        '@vuepress/plugin-medium-zoom', ['@snowdog/vuepress-plugin-pdf-export', {
            puppeteerLaunchOptions: {
                // slowMo: 1000
            },
            sorter: function(a, b) {
                if (a.relativePath == "index.md") return -1;
                if (b.relativePath == "index.md") return 1;

                var children1 = a.relativePath.split('.md')[0];
                var children2 = b.relativePath.split('.md')[0];

                return sideBarChildren.indexOf(children1) > sideBarChildren.indexOf(children2) ? 1 : -1;
            }
        }],
        ['vuepress-plugin-code-copy', {
            color: '#ffffff'
        }]
    ]
}