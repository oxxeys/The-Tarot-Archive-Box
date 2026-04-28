const mongoose=require("mongoose")
const {Schema, model} = mongoose

const dataSchema = new Schema({
    boxID: String,
    data : [Number],
})

const boxData = model("data", dataSchema)

//add box data to database 
async function addNewBoxData(boxID, data){

        let newBoxData = {
            boxID: boxID,
            data: data
        }
        await boxData.create(newBoxData)
        return true
}


module.exports={
    addNewBoxData
}