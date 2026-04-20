const mongoose=require("mongoose")
const {Schema, model} = mongoose


const userSchema = new Schema({
    username: String,
    password: String,
})


const userData = model("users", userSchema)


async function checkUser(username, password){
    //return bool if user is there

    let found = null
    found = await userData.findOne({username:username}).exec()

    //let found=userData.find(thisUser=>thisUser.username==username) // arrow function that assigns each item in userData to "thisUser" temporarily, then checks if the username is equal to the username the client just inputted to the form 
    if(found){ // js treats a variable with a value as true, var with no value is false
        return found.password==password // checks password - if it is same then it will return true
    } else{
        return false
    }
}
