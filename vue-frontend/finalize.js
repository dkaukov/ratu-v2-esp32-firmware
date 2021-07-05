const { gzip } = require('@gfx/zopfli');
const FS = require('fs');
const path = require('path');

const BUNDLE_JS = FS.readFileSync(path.resolve(__dirname, './dist/js/app.js'));

const HTML = `
<!DOCTYPE html>
<html lang=en>
<head>
    <meta charset=utf-8>
    <meta http-equiv=X-UA-Compatible content="IE=edge">
    <meta name=viewport content="width=device-width,initial-scale=1">
    <link href="data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOV//8Dlf//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACA5X//wOV//8AAAAAAAAAAAAAAAAAAAAAAAAAPwAAAAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIDlf//A5X//wAAAAAAAAAAAAAAOQOV//8Dlf//AAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgOV//8Dlf//AAAAOQOV//8Dlf//A5X//wOV//8AAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACA5X//wOV//8Dlf//AAAABgAAAAADlf//A5X//wAAAAEAAAAAA5X//wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAIDlf//AAAAAgAAAAAAAAAAAAAAAQOV//8Dlf//A5X//wOV//8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARA5X//wOV//8Dlf//AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQA5X//wOV//8Dlf//A5X//wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA//8AAP//AAD//wAA//8AAJ//AADP/wAA5z8AAPIfAAD4zQAA/eEAAP/xAAD/4QAA//8AAP//AAD//wAA//8AAA==" rel="icon" type="image/x-icon" />
</head>
<body>
    <noscript>
        <strong>We're sorry but ATU-DASH doesn't work properly without JavaScript enabled. Please enable it to continue.</strong>
    </noscript>
    <div id=app></div>
    <script defer>${BUNDLE_JS}</script>
</body>
</html>
`;

function chunkArray(myArray, chunk_size) {
  let index = 0;
  const arrayLength = myArray.length;
  const tempArray = [];
  for (index = 0; index < arrayLength; index += chunk_size) {
    let myChunk = myArray.slice(index, index + chunk_size);
    // Do something if you want with the group
    tempArray.push(myChunk);
  }
  return tempArray;
}

function addLineBreaks(buffer) {
  let data = '';
  const chunks = chunkArray(buffer, 30);
  chunks.forEach((chunk, index) => {
    data += chunk.join(',');
    if (index + 1 !== chunks.length) {
      data += ',\n';
    }
  });
  return data;
}


gzip(HTML, { numiterations: 15, blocksplittingmax: 100 }, (err, output) => {
  if (err) {
    /* eslint-disable no-console */
    return console.error(err);
    /* eslint-enable no-console */
  }

  const FILE = `#ifndef DashWebpage_h
#define DashWebpage_h
const uint32_t DASH_HTML_SIZE = ${output.length};
const uint8_t DASH_HTML[] PROGMEM = { 
${addLineBreaks(output)} 
};
#endif
`;

  FS.writeFileSync(path.resolve(__dirname, '../src/network/webpage.h'), FILE);
  /* eslint-disable no-console */
  console.log(`[COMPRESS] Compressed Build Files to webpage.h: ${ (output.length/1024).toFixed(2) }KB`);
  /* eslint-enable no-console */
});