.pragma library

var getLoginFlowUrl = function(host) {
    if(!host || host.trim() === "") {
        throw "Invalid host";
    }

    if(host.indexOf('https://') === -1 && host.indexOf('http://') === -1) {
        host = 'https://' + host;
    }

    if(host.substring(host.length - 1) !== '/') {
        host = host + '/';
    }

    return host + "index.php/login/flow";
}

var triggerLoginFlow = function(host, webView) {
    var url = getLoginFlowUrl(host);

    fetch(url, {
        headers: { "OCS-APIREQUEST": "true" },
    })
        .then(function(response) {
            if(!response.ok) {
                throw 'Querying login flow endpoint failed';
            }
            console.log(response.status);
            if(response.status !== 200) {
                throw 'Nextcloud instance gave an unexpected response';
            }
            return response.blob()
        })
        .then(function(html) {
            console.log(html);
            webView.loadHtml(html, url);
        });
}
