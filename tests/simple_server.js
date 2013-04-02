var http = require('http'),
    hcal = require('../build/Debug/hcal');

http.createServer(function (req, res) {
    res.writeHead(200, {'Content-Type': 'text/plain'});
    var from = new Date(2013, 0, 7),
        to = new Date(2013, 0, 14);
        
    hcal.getEvents(from, to, 1, function(err, evts){
        if (err){
            console.log(err);
        }
        else{
            retval = [];
            var len = evts.length, i = 0;
            for (; i < len; ++i){
                retval[i] = {
                    id: evts[i].id(),
                    text: evts[i].description(),
                    start_date: evts[i].start(),
                    end_date: evts[i].end(),
                    room_id: evts[i].roomId()
                };
                console.log(evts[i].description() + " - " + evts[i].start());
            }
            res.end(JSON.stringify(retval));
        }
    });
    
}).listen(1337, "127.0.0.1");
console.log('Server running at http://127.0.0.1:1337/');