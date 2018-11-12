var obj =
[
  {
  "group": "",
  "name": "FN",
  "values": ["Simon Perreault"]
  }, {
    "group": "",
    "name": "N",
    "values": ["Perreault", "Simon", "", "", "ing. jr,M.Sc."]
  }, {
    "group": "",
    "name": "GENDER",
    "values": ["M"]
  }, {
    "group": "",
    "name": "LANG",
    "values": ["fr"]
  }, {
    "group": "",
    "name": "LANG",
    "values": ["en"]
  }, {
    "group": "",
    "name": "ORG",
    "values": ["Viagenie"]
  }, {
    "group": "",
    "name": "ADR",
    "values": ["", "Suite D2-630", "2875 Laurier", "Quebec", "QC", "G1V 2M2", "Canada"]
  }, {
    "group": "",
    "name": "TEL",
    "values": ["tel:+1-418-656-9254", "ext=102"]
  }, {
    "group": "",
    "name": "TEL",
    "values": ["tel:+1-418-262-6501"]
  }, {
    "group": "",
    "name": "EMAIL",
    "values": ["simon.perreault@viagenie.ca"]
  }, {
    "group": "",
    "name": "GEO",
    "values": ["geo:46.772673,-71.282945"]
  }, {
    "group": "",
    "name": "KEY",
    "values": ["http://www.viagenie.ca/simon.perreault/simon.asc"]
  }, {
    "group": "",
    "name": "TZ",
    "values": ["-0500"]
  }, {
    "group": "",
    "name": "URL",
    "values": ["http://nomis80.org"]
  }, {
    "isText": false,
    "date": "--0203",
    "time": "",
    "text": "",
    "isUTC": false
  }, {
    "isText": false,
    "date": "20090808",
    "time": "143000",
    "text": "",
    "isUTC": false
  }
]

$( document ).ready(function() {
  var ulTag = '<ul class="collection">';
  var ulClosing = '</ul>';
  var liClosing = '</li>';
  var string = document.getElementById('fileLog').innerHTML;
  string = string + ulTag;
  var online = document.getElementById('cardView').innerHTML;
  online = online + ulTag;
  var offline = document.getElementById('status').innerHTML;
  offline = offline + ulTag;
  var listElement = "";
  for (var i = 0; i < obj.length; i++) {
    if (obj[i].stream != null) {
      listElement = listElement + '<li class="collection-item avatar" id="online_client">';
      listElement = listElement + ' <img src=' + obj[i].stream.logo + 'alt="" class="circle">';
      listElement = listElement + '<span class="title streamName">'+ obj[i].stream.display_name + '</span>';
      listElement = listElement + '<p class="overflow ellipsis">' + obj[i].stream.status + '</p>';
      listElement = listElement + '<a href="' + obj[i].stream.url + '"class="secondary-content"><i class="material-icons">Online</i></a>';
      string = string + listElement;
      online = online + listElement;
      listElement = "";
    }
    if (obj[i].stream == null) {
      if (obj[i].display_name != undefined) {
        listElement = listElement + '<li class="collection-item avatar" id="offline_client">';
        listElement = listElement + ' <img src=' + "images/yuna.jpg" + ' alt="" class="circle">';
        listElement = listElement + '<span class="title streamName">'+ obj[i].display_name + '</span>';
        listElement = listElement + '<p class="overflow ellipsis">' + "Oops! Seems like you caught me when I am Offline." + '</p>';
        listElement = listElement + '<a href="#" class="secondary-content"><i class="material-icons">Offline</i></a>'; 
        string = string + listElement;
        offline = offline + listElement;
        listElement = "";
      }
    }
    document.getElementById('fileLog').innerHTML = string; 
    document.getElementById('cardView').innerHTML = online;
    document.getElementById('status').innerHTML = offline;
  }
});