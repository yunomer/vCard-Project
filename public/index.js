$( document ).ready(function() {

  $("#status-chip-remove").click(function() {
    document.getElementById('status-chip').innerHTML = "";
  });

  $('select[name="dropdown"]').change(function(){
    let userChoice =$(this).val();
    $.ajax({
      type: "GET",
      url: 'serverDataDump',
      data: "",
      success: function(response){
        displayData(response, userChoice);
      },
      error: function(err) {
        alert("Oops! Error: " + err);
      }
    });
  });

  document.getElementById("status-chip-remove").onclick = function() {
    delete sessionStorage.chips
  }

  $.ajax({
    type: "GET",
    url: 'serverDataDump',
    data: "",
    success: function(response){
      distributer(response)
      document.getElementById('uploadButton').onclick = function () {
        let errors = []
        var filename = $('#fileInput').val();
        htmlData = `Uploaded File: ${filename.slice(12, filename.length)}`
        errors.push(htmlData);
        var oldItems = JSON.parse(sessionStorage.getItem('chips')) || [];
        oldItems.push(errors);
        sessionStorage.setItem('chips', JSON.stringify(oldItems));
        createChips();
      };
    },
    error: function(err) {
      alert("Oops! Error: " + err);
    }
  });
})

function distributer(response) {
  let string = JSON.stringify(response);
  let object = JSON.parse(string);

  let file_names = object.file_names;
  let file_data = object.file_data;

  fileLog(file_names, file_data);
  cardView(file_names, file_data);
  createChips();

}

function fileLog(file_names, file_data) {
  var errors = []
  var content = `
                  <table class="responsive-table centered highlight">
                    <thead>
                      <tr>
                          <th>File name (click to download)</th>
                          <th>Individual's Name</th>
                          <th>Additional properties</th>
                      </tr>
                    </thead>
                    <tbody>`;
  var uploadButton = `
                  <tr>
                    <td>
                      <form method='post' action='upload' enctype="multipart/form-data">
                        <input type='file' name='uploadFile' id="fileInput" required/>
                        <input type='submit' id="uploadButton">
                      </form>
                    </td>
                  </tr>`;
  var status = ``;
  var file_names_len = file_names.length;

  var validFiles = 0
  
  for(let i = 0; i < file_names_len; i++) {
    try {
      var data = JSON.parse(file_data[i]);
      fnValue = data[0]["values"];
      let returnString = fileLog_item_creator(file_names[i], file_data[i]);
      content = content + returnString;
      validFiles++;
    }
    catch(err) {
      let stringToAdd = `ERROR! ${file_names[i]} is: ${file_data[i]}`;
      try {
        let sessionStorage_ = JSON.parse(sessionStorage.chips);
        let sessionStorage_len = sessionStorage_.length;
  
        let exists = -1;
  
        for (let i = 0; i < sessionStorage_len; i++) {
          let tempResult = stringToAdd.localeCompare(sessionStorage_[i]);
          if (tempResult === 0) {
            exists = 0;
          }
        }
        if (exists !== 0) {
          errors.push(`ERROR! ${file_names[i]} is: ${file_data[i]}`);
        }
      } catch(err) {
        errors.push(`ERROR! ${file_names[i]} is: ${file_data[i]}`);
      }
    }
  }

  if (validFiles === 0) { //If there are no valid files
    content = "<H3>No Files</H3>"
    content = content + uploadButton;
    document.getElementById('fileLog').innerHTML = content;
    document.getElementById('status-chip').innerHTML = status;
    let errors_len = errors.length;

    if (errors_len !== 0) {
      var oldItems = JSON.parse(sessionStorage.getItem('chips')) || [];
      oldItems.push(errors);
      sessionStorage.setItem('chips', JSON.stringify(oldItems));
    }
    return;
  }
  content = content + uploadButton;
  content = content + `</tbody></table>`
  document.getElementById('fileLog').innerHTML = content;
  document.getElementById('status-chip').innerHTML = status;

  let errors_len = errors.length;

  if (errors_len !== 0) {
    var oldItems = JSON.parse(sessionStorage.getItem('chips')) || [];
    oldItems.push(errors);
    sessionStorage.setItem('chips', JSON.stringify(oldItems));
  }
}

function cardView(file_names, file_data) {
  var content = `
  <table class="responsive-table centered highlight">
    <thead>
      <tr>
          <th>Property #</th>
          <th>Group</th>
          <th>Name</th>
          <th>Values</th>
      </tr>
    </thead>
    <tbody>`;
  let file_names_len = file_names.length;
  let dropdown_item = `<option value="" disabled selected>Choose your option</option>`;
  for(let i = 0; i < file_names_len; i++) {
    try {
      let data = JSON.parse(file_data[i]);
      fnValue = data[0]["values"];
      let returnString = `<option value="${i+1}">${file_names[i]}</option>`;
      dropdown_item = dropdown_item + returnString;
    }
    catch(err) {
    }
  }
  document.getElementById('cardNames').innerHTML = dropdown_item;
  document.getElementById('selectedCard').innerHTML = content;
  return;
}

function fileLog_item_creator(file_name, file_data) {
  try{
    let data = JSON.parse(file_data);
    fnValue = data[0]["values"];
    let number_of_dates = 0;
    let number_of_JSONs = data.length;

    for (let i = 0; i < number_of_JSONs; i++) {
      if (data[i]["text"]) {
        number_of_dates++;
      }
    }
    let additionalProps = number_of_JSONs - number_of_dates - 1;
    let additionalProps_placeholder = 0;
    if (additionalProps > 0) {
      return `<tr>
      <td>
        <a class="waves-effect waves-light btn-small" href="/uploads/${file_name}">
          <i class="material-icons right">
            file_download
          </i>
          ${file_name}
        </a>
      </td>
      <td>${fnValue}</td>
      <td>${additionalProps}</td>
      </tr>`;
    }
    return `<tr>
      <td>
        <a class="waves-effect waves-light btn-small" href="/uploads/${file_name}">
          <i class="material-icons right">
          file_download
          </i>
          ${file_name}
        </a>
      </td>
      <td>${fnValue}</td>
      <td>${additionalProps_placeholder}</td>
      <td>
    </tr>`;
  } catch(err) {
    return;
  }
}

function displayData(response, userChoice) {
  let returnString = `
  <table class="responsive-table centered highlight">
    <thead>
      <tr>
          <th>Property #</th>
          <th>Group</th>
          <th>Name</th>
          <th>Values</th>
      </tr>
    </thead>
    <tbody>`;
  let string = JSON.stringify(response);
  let object = JSON.parse(string);
  let file_names = object.file_names;
  let file_data = object.file_data;

  let parseData = JSON.parse(file_data[userChoice-1]);
  let parseData_len = parseData.length;

  for (let i = 0; i < parseData_len; i++) {
    let name = parseData[i]["name"];
    let values = parseData[i]["values"];
    let group = parseData[i]["group"];

    if ((name != undefined) || (values != undefined)) {
      let temp = `<tr>
      <td>${i+1}</td>
      <td>${group}</td>
      <td>${name}</td>
      <td>${values}</td>
      </tr>`;
      returnString = returnString + temp;
    }
  }

  document.getElementById('selectedCard').innerHTML = returnString;

}

function createChips() {
  let raw_data = sessionStorage.getItem('chips');
  if (raw_data === null) {
    return;
  }
  
  let htmlData = ``;

  let processed = JSON.parse(raw_data);

  for(let i = 0; i < processed.length; i++) {
    htmlData = htmlData + `  
    <div class="chip file_name">
    ${processed[i][0]}
    <i class="close material-icons">close</i>
    </div>`
  }
  document.getElementById('status-chip').innerHTML = htmlData;
}