const http = require('http');
const url = require('url');
const fs = require('fs');
const createHTML = require('create-html');
var filignore = ["requestservice.html", "runserver.bat", "ServerFactorem.zip", "servermain.js", "mainstylesheet.css"];
var html;
var ip4 = '192.168.1.217';
var wiredip4 = '10.235.0.1';
const { COPYFILE_EXCL } = fs.constants;
var portnum = 8080;
var contentType = {'Content-Type':'text/html'};

function arrayIncludes (string, array) {
    for(var i = 0; i < 5; i++) {
        if(string.includes(array[i])) return true;
    }
    return false;
}

function processContentType (filename) {
    if(arrayIncludes(filename, filignore) === false) {
        if(filename.includes('.js') === true) {
            contentType["Content-Type"] = 'text/javascript';
        }
        if(filename.includes('.html') === true || filename.includes('.txt') === true) {
            contentType["Content-Type"] = 'text/html';
        }
        if(filename.includes('.css') === true) {
            contentType["Content-Type"] = 'text/css';
        }
        if(filename.includes('.jpg') === true || filename.includes('.JPG') === true || filename.includes('.png') === true || filename.includes('.PNG') === true || filename.includes('.bmp') === true || filename.includes('.BMP')) {
            contentType["Content-Type"] = 'image';
        }
        if(filename.includes('.mp4') === true || filename.includes('.MP4') === true || filename.includes('.wmv') === true || filename.includes('.WMV') === true || filename.includes('.avi') === true || filename.includes('.AVI')) {
            contentType["Content-Type"] = 'video';
        }
        if(filename.includes('.exe') === true) {
            contentType["Content-Type"] = 'executable';
        }
        if(filename.includes('.apk') === true || filename.includes('.APK') === true) {
            contentType["Content-Type"] = 'apk';
        }
        return contentType["Content-Type"];
    }
    else return 'IGNORE';
}

function createHTMLContent (filelist, size) {
    let htmlContent = `<h1>Main Server Navigational Page</h1>\n<table>\n<tr>\n<th>File Names</th>\n<th>File Type</th>\n<th>Options</th>\n</tr>`;
    for(var i = 0; i < size; i++) {
        let ct = processContentType(filelist[i]);
        if(ct === 'image')
            htmlContent = htmlContent.concat(`\n<tr>\n<td><img src="${filelist[i]}" width="100px"/></td>\n<td>${processContentType(filelist[i])}</td>\n<td><a href="http://${ip4}:8080/${filelist[i]}">View</a><a style="margin-left:10px;" href="http://${ip4}:8080/${filelist[i]}" download="${filelist[i]}">Download</a></td>\n</tr>`);
        else if(ct === 'IGNORE') {
            //We want the css to take effect, so we have to set the content type to text/css
            contentType["Content-Type"] = 'text/css';
            htmlContent = htmlContent.concat(`\n</table>`);
            return htmlContent;
        }
        else
            htmlContent = htmlContent.concat(`\n<tr>\n<td>${filelist[i]}</td>\n<td>${processContentType(filelist[i])}</td>\n<td><a href="http://${ip4}:8080/${filelist[i]}">View</a><a style="margin-left:10px;" href="http://${ip4}:8080/${filelist[i]}" download="${filelist[i]}">Download</a></td>\n</tr>`);
    }
    htmlContent = htmlContent.concat(`\n</table>`);
    //console.log(`HTML Content: ${htmlContent}`);
    return htmlContent;
}

var source = function (req, res) {
    var path = url.parse(req.url, true);
    var filelist = new Array(100);
    var fn = "." + path.pathname;
    let index = 0;
    let htmlContent;
    if(req.url.includes(".")) {
        /*console.log();
        console.log('----------------');
        console.log("Filepath: " + fn);  
        console.log('Server storage/directory: ');*/
        fs.readdirSync(__dirname).forEach(file => {
            //console.log("File accessible by server: " + file);
            filelist[index] = file;
            index++;
        });
        htmlContent = createHTMLContent(filelist, index);
        html = createHTML({
            title: 'Server Index',
            lang: 'en',
            css: 'mainstylesheet.css',
            body: htmlContent
        });
        fs.writeFile('serverindex2.html', html, function(err) {
            if (err) console.log(err);
        });
        processContentType (fn);
        if(fn != './') {
            fs.readFile(fn, function(err, data) {
            if (err) {
                res.writeHead(404, contentType);
                return res.end("404 Not Found");
            } 
            res.writeHead(200, contentType);
            res.write(data);
            return res.end();
            });
        }
        else {
            fs.readFile('serverindex2.html', function(err, data) {
                if (err) {
                res.writeHead(404, contentType);
                return res.end("404 Not Found");
                } 
                res.writeHead(200, {'Content-Type':'text/html'});
                res.write(data);
                return res.end();
            });
        }
    }
    else {
        if(req.method === 'POST' && req.url === '/echo') {
            req.pipe(res);
        }
        else {
            res.statusCode = 404;
            res.end();
        }
    }
}
http.createServer(source).listen(portnum, ip4);

function callback(err) {
    if (err) throw err;
}
