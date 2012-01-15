function send_to_server(response, callback) {
    url = "/fMBTweb." + response;

    var xmlHttp = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("MSXML2.XMLHTTP.3.0");

    xmlHttp.onreadystatechange = function() {
        if (xmlHttp.readyState == 4) callback(xmlHttp);
    }
    xmlHttp.open("GET", url, true);
    xmlHttp.send();
}

function eval_response(xmlHttp) {
    try {
	eval_result = eval(xmlHttp.responseText);
    } catch (e) {
	eval_result = "js eval error";
    }
    send_to_server(eval_result, eval_response);
}

send_to_server(null, eval_response);
