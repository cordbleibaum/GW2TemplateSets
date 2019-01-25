# GW2TemplateSets
Addon for saving arcdps buildtemplates in sets.

#### How it works:
Copies ArcDPS buildtemplate folder into separate folders, and switches the content of the ArcDPS buildtemplate folder for loading sets

#### Current Limitations:
- You can't use most special chars in template set names, because the names are used as folder names at the moment

#### *This is not completely tested yet, you might loose buildtemplates if there is still a bug. And possibly in catastrophic cases more data (unlikely, but in theory possible, when deleting folders, that such a bug exists)*

## Build-Dependencies
- Boost