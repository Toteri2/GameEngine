import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

/**
 * Creating a sidebar enables you to:
 - create an ordered group of docs
 - render a sidebar for each doc of that group
 - provide next/previous navigation

 The sidebars can be generated from the filesystem, or explicitly defined here.

 Create as many sidebars as you want.
 */
const sidebars: SidebarsConfig = {
  tutorialSidebar: [
    'intro',
    'project-overview',
    {
      type: 'category',
      label: 'Tutorials',
      items: [
        'tutorials/getting-started',
      ],
    },
    {
      type: 'category',
      label: 'Engine Systems',
      items: [
        'engine/architecture',
        'engine/rendering-system',
        'engine/input-system',
        'engine/scripting-system',
        'engine/level-system',
        'engine/audio-system',
      ],
    },
    {
      type: 'category',
      label: 'Network',
      items: [
        'network/protocol',
        'network/network-architecture',
      ],
    },
    {
      type: 'category',
      label: 'Technical',
      items: [
        'technical/techno',
        'technical/editor',
      ],
    },
    {
      type: 'category',
      label: 'Development',
      items: [
        'development/build',
      ],
    },
  ],
};

export default sidebars;
