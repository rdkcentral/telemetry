/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

const https = require('node:https');
const path = require('node:path');
const fs = require('node:fs');
const url = require('node:url');

const options = {
  key: fs.readFileSync(path.join('/etc/xconf/certs/mock-xconf-server-key.pem')),
  cert: fs.readFileSync(path.join('/etc/xconf/certs/mock-xconf-server-cert.pem')),
  port: 50050
};

let save_request = false;
let savedrequest_json={};

/**
 * Function to read JSON file and return the data
 */
function readJsonFile(count) {
  if(count == 0){
    var filePath = path.join('/etc/xconf', 'xconf-dcm-response.json');
  }
  else{
    var filePath = path.join('/etc/xconf', 'xconf-dcm-response1.json');

  }
  try {
    const fileData = fs.readFileSync(filePath, 'utf8');
    return JSON.parse(fileData);
  } catch (error) {
    console.error('Error reading or parsing JSON file:', error);
    return null;
  }
}  

function handleT2DCMSettings(req, res, queryObject, file) {
  let data = '';
  req.on('data', function(chunk) {
    data += chunk;
  });
  req.on('end', function() {
    console.log('Data received: ' + data);
  });

  if (save_request) {
    savedrequest_json[new Date().toISOString()] = { ...queryObject };
  }

  res.writeHead(200, {'Content-Type': 'application/json'});
  res.end(JSON.stringify(readJsonFile(file)));
  return;
}

function handleAdminSet(req, res, queryObject) {
  if (queryObject.saveRequest === 'true') {
    save_request = true;
  } else if (queryObject.saveRequest === 'false') {
    save_request = false;
    savedrequest_json={};
  }
}

function handleAdminGet(req, res, queryObject) {
  if (queryObject.returnData === 'true') {
    res.writeHead(200, {'Content-Type': 'application/json'});
    res.end(JSON.stringify(savedrequest_json));
    return;
  }
  res.writeHead(200);
  res.end('Admin Get Unknown Request');
}

/**
 * Handles the incoming request and logs the data received
 * @param {http.IncomingMessage} req - The incoming request object
 * @param {http.ServerResponse} res - The server response object
 */
function requestHandler(req, res) {
  const queryObject = url.parse(req.url, true).query;
  console.log('Query Object: ' + JSON.stringify(queryObject));
  console.log('Request received: ' + req.url);
  console.log('json'+JSON.stringify(savedrequest_json));
  if (req.method === 'GET') {

    if (req.url.startsWith('/loguploader/getT2DCMSettings')) {

      return handleT2DCMSettings(req, res, queryObject,0); 

    }
    else if (req.url.startsWith('/adminSupportSet')) {

      handleAdminSet(req, res, queryObject);

    }
    else if (req.url.startsWith('/adminSupportGet')) {

      return handleAdminGet(req, res, queryObject);

    }
    else if (req.url.startsWith('/loguploader404/getT2DCMSettings')) {
      res.writeHead(404);
      res.end("404 No Content");
      return;
    }
    else if (req.url.startsWith('/loguploader1/getT2DCMSettings')){
      return handleT2DCMSettings(req, res, queryObject,1); 
    }
}
else if (req.method === 'POST') {
  // TO BE IMPLEMENTED
  if (req.url.startsWith('/updateT2DCMSettings')) {
    let body = '';
    req.on('data', chunk => {
      body += chunk.toString();
    });
    req.on('end', () => {
      const postData = JSON.parse(body);
      const redirect_json = { ...postData };
      redirect_json[new Date().toISOString()] = { ...queryObject };;// Example of adding a timestamped entry

      res.writeHead(200, {'Content-Type': 'application/json'});
      res.end(JSON.stringify(redirect_json));
  });
  }

}
  res.writeHead(200);
  res.end("Server is Up Please check the request....");
}
const serverInstance = https.createServer(options, requestHandler);
serverInstance.listen(
  options.port,
  () => {
    console.log('XCONF DCM Mock Server running at https://localhost:50050/');
  }
);