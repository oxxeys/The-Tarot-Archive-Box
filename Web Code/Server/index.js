const express = require("express");
const path = require("path");
const app = express();

//Connect to mongo
const dotenv = require('dotenv').config()
const mongoUsername=process.env.mongoUsername
const mongoPassword=process.env.mongoPassword
const mongoAppName=process.env.mongoAppName

const DBConnection = `mongodb+srv://${mongoUsername}:${mongoPassword}@mysticreader.u8qzncq.mongodb.net/${mongoAppName}?retryWrites=true&w=majority` 
const mongoose = require("mongoose");
const session = require("express-session");
mongoose.connect(DBConnection)

// DB models
const Users = require("./models/users.js");
const { request } = require("http");


// Set up a express session
app.use(session({
  secret: "KEY",
  resave: false,
  saveUninitialized: false,
  cookie:{maxAge: 5*60*1000 }
}))




//serve files from /public 
app.use(express.static('public'))

// allow form data to be used
app.use(express.urlencoded({extended: false}))

// allow json data to be recieved
app.use(express.json());

const port = process.env.PORT || 3000; // set like this to work with Render
app.listen(port, () => console.log(`Server running on ${port}`));

function checkIfLoggedIn(request, response, nextAction){
  if(request.session){
        if(request.session.username){
            nextAction()
        }else{
            request.session.destroy()
            response.sendFile(path.join(__dirname, '/views', 'notloggedin.html'))
        }
    }
    else{
        request.session.destroy()
        response.sendFile(path.join(__dirname, '/views', 'notloggedin.html'))
    } 
}

app.post(`/login`, async (request, response)=>{
  if(await userModel.checkUser(request.body.username, request.body.password)){
        request.session.username=request.body.username
        response.sendFile(path.join(__dirname, '/views', 'app.html'))
    } else{
        response.sendFile(path.join(__dirname, '/views', 'notloggedin.html'))
    }
})

app.get('/login', (request, response)=>{
    response.sendFile(path.join(__dirname, '/views', 'login.html'))
})


app.get('/app', checkIfLoggedIn, (req, res) => {
  res.send(`Welcome ${req.session.username}`);
});

// Data is sent from ESP32
app.post("/data", (req, res) => {
  console.log(req.body);
  res.send("OK");

  //Send data to mongo, store in db 

});
 